// server_select.c
// 빌드: gcc -Wall -Wextra -O2 -o server server_select.c net.c clients.c
// 실행: ./server 12345

#define _POSIX_C_SOURCE 200112L
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <string.h>
#include <unistd.h>

#include "net.h"
#include "clients.h"

// 콜백: 완성된 프레임(=한 메시지)이 도착하면 호출된다.
static void on_message(Client* cs, int sender_idx,
                       const unsigned char* payload, size_t len, void* user) {
    (void)user;

    // 간단한 텍스트 명령 파서: "/nick foo"
    // payload는 널종료가 없을 수 있으므로 임시 버퍼에 카피 후 처리
    char tmp[INBUF_CAP + 1];
    size_t n = (len < INBUF_CAP) ? len : INBUF_CAP;
    memcpy(tmp, payload, n);
    tmp[n] = '\0';

    // 앞공백 제거
    char* p = tmp;
    while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') p++;

    if (strncmp(p, "/nick ", 6) == 0) {
        const char* newname = p + 6;
        // 뒤쪽 공백/개행 제거
        size_t L = strlen(newname);
        while (L > 0 && (newname[L-1] == '\r' || newname[L-1] == '\n' || newname[L-1] == ' ' || newname[L-1] == '\t')) {
            L--;
        }
        if (L >= sizeof(cs[sender_idx].name)) L = sizeof(cs[sender_idx].name) - 1;
        memcpy(cs[sender_idx].name, newname, L);
        cs[sender_idx].name[L] = '\0';

        // 확인 메시지를 보낸다(개별 전송)
        char ok[128];
        int m = snprintf(ok, sizeof(ok), "[server] nick set to '%s'\n", cs[sender_idx].name[0] ? cs[sender_idx].name : "(empty)");
        if (m < 0) m = 0;
        send_frame(cs[sender_idx].fd, ok, (uint32_t)m);
        return;
    }

    // 일반 메시지는 브로드캐스트
    clients_broadcast_text_framed(cs, sender_idx, (const unsigned char*)tmp, n);
}

int main(int argc, char** argv) {
    signal(SIGPIPE, SIG_IGN); // 닫힌 소켓에 send 시 프로세스가 죽지 않도록

    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        return 1;
    }
    unsigned short port = (unsigned short)atoi(argv[1]);

    int lfd = setup_listen_socket(port);
    printf("[week3] listening on port %u (select + framed protocol)\n", port);

    Client clients[MAX_CLIENTS];
    clients_init(clients);

    for (;;) {
        fd_set rfds;
        clients_build_readfds(clients, lfd, &rfds);
        int maxfd = clients_max_fd(clients, lfd);

        int ready = select(maxfd + 1, &rfds, NULL, NULL, NULL);
        if (ready < 0) {
            if (errno == EINTR) continue;
            perror("select");
            break;
        }

        // 1) 새 연결
        if (FD_ISSET(lfd, &rfds)) {
            struct sockaddr_in cli;
            socklen_t clen = sizeof(cli);
            int cfd = accept(lfd, (struct sockaddr*)&cli, &clen);
            if (cfd < 0) {
                if (errno != EINTR) perror("accept");
            } else {
                int idx = clients_add(clients, cfd, &cli);
                if (idx < 0) {
                    fprintf(stderr, "too many clients; rejecting\n");
                    close(cfd);
                } else {
                    printf("client connected: %s:%u (fd=%d, slot=%d)\n",
                           clients[idx].ip, clients[idx].port, cfd, idx);

                    // 환영 메시지(개별 프레임)
                    char hello[128];
                    int m = snprintf(hello, sizeof(hello),
                                     "[server] welcome! use '/nick yourname' to set nickname.\n");
                    if (m < 0) m = 0;
                    send_frame(cfd, hello, (uint32_t)m);
                }
            }
            if (--ready <= 0) continue;
        }

        // 2) 기존 클라이언트 읽기 (프레이밍 누적 처리)
        for (int i = 0; i < MAX_CLIENTS && ready > 0; i++) {
            int cfd = clients[i].fd;
            if (cfd == -1) continue;
            if (!FD_ISSET(cfd, &rfds)) continue;

            unsigned char buf[4096];
            ssize_t n = recv(cfd, buf, sizeof(buf), 0);
            if (n == 0) {
                printf("client disconnected: %s:%u (fd=%d, slot=%d)\n",
                       clients[i].ip, clients[i].port, cfd, i);
                clients_remove(clients, i);
            } else if (n < 0) {
                if (errno != EINTR) {
                    perror("recv");
                    clients_remove(clients, i);
                }
            } else {
                // 받은 바이트를 누적 → 완성된 프레임마다 on_message() 호출
                clients_feed_frames(clients, i, buf, (size_t)n, on_message, NULL);
            }
            --ready;
        }
    }

    close(lfd);
    return 0;
}
