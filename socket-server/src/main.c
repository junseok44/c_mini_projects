// server_select.c
// 빌드: gcc -Wall -Wextra -O2 -o server server_select.c net.c clients.c
// 실행: ./server 12345

#define _POSIX_C_SOURCE 200112L
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <unistd.h>

#include "net.h"
#include "clients.h"

int main(int argc, char** argv) {
    signal(SIGPIPE, SIG_IGN); // 닫힌 소켓에 send 시 프로세스가 죽지 않도록

    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        return 1;
    }
    unsigned short port = (unsigned short)atoi(argv[1]);

    int lfd = setup_listen_socket(port);
    printf("[day2] listening on port %u (select-based)\n", port);

    Client clients[MAX_CLIENTS];
    clients_init(clients);

    for (;;) {
        fd_set rfds;
        // cs에 유효한 fd들을 rfds에 넣어주는 것.
        clients_build_readfds(clients, lfd, &rfds);
        int maxfd = clients_max_fd(clients, lfd);

        // 이벤트가 하나라도 있나요?
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
                }
            }
            if (--ready <= 0) continue;
        }

        // 2) 기존 클라이언트 읽기
        for (int i = 0; i < MAX_CLIENTS && ready > 0; i++) {
            int cfd = clients[i].fd;
            if (cfd == -1) continue;
            if (!FD_ISSET(cfd, &rfds)) continue;

            char buf[4096];
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
                // 수신 데이터를 다른 모든 클라에게 브로드캐스트
                clients_broadcast(clients, cfd, buf, (size_t)n);
            }
            --ready;
        }
    }

    close(lfd);
    return 0;
}
