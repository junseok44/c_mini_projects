// clients.c
#define _POSIX_C_SOURCE 200112L
#include "clients.h"
#include "net.h"

#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>

void clients_init(Client* cs) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        cs[i].fd = -1;
        cs[i].in_used = 0;
        cs[i].name[0] = '\0';
    }
}

int clients_add(Client* cs, int cfd, struct sockaddr_in* cli) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (cs[i].fd == -1) {
            cs[i].fd   = cfd;
            cs[i].addr = *cli;
            inet_ntop(AF_INET, &cli->sin_addr, cs[i].ip, sizeof(cs[i].ip));
            cs[i].port = (unsigned short)ntohs(cli->sin_port);
            cs[i].in_used = 0;
            cs[i].name[0] = '\0'; // 기본: 닉네임 없음
            return i;
        }
    }
    return -1;
}

void clients_remove(Client* cs, int idx) {
    if (cs[idx].fd != -1) {
        close(cs[idx].fd);
        cs[idx].fd = -1;
        cs[idx].in_used = 0;
        cs[idx].name[0] = '\0';
    }
}

int clients_find_by_fd(Client* cs, int fd) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (cs[i].fd == fd) return i;
    }
    return -1;
}

void clients_build_readfds(Client* cs, int lfd, fd_set* rfds) {
    FD_ZERO(rfds);
    FD_SET(lfd, rfds);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (cs[i].fd != -1) FD_SET(cs[i].fd, rfds);
    }
}

int clients_max_fd(Client* cs, int lfd) {
    int maxfd = lfd;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (cs[i].fd > maxfd) maxfd = cs[i].fd;
    }
    return maxfd;
}

static void drain_frames_from_inbuf(Client* cs, int sender_idx, on_message_cb cb, void *user) {
    unsigned char *b = cs[sender_idx].inbuf;
    size_t used = cs[sender_idx].in_used;

    size_t offset = 0;
    while (used - offset >= 4) {
        // 4바이트.
        uint32_t networkOrder;
        // message 사이즈는?
        memcpy(&networkOrder, b + offset, 4);
        // 이게 본문의 길이임.
        size_t len = ntohl(networkOrder);

        if (len > MAX_FRAME_SIZE) {
            fprintf(stderr, "frame too large from fd=%d (%zu bytes). drop client.\n", cs[sender_idx].fd, len);
            clients_remove(cs, sender_idx);
            return;
        }

        if (used - offset < 4 + len) {
            break;
        }

        const unsigned char* payload = b + offset + 4;
        if (cb) cb(cs, sender_idx, payload, len, user);
        offset += 4 + len;
    }

    if (offset > 0) {
        size_t remain = used - offset;
        if (remain > 0) memmove(b, b+offset, remain);
        cs[sender_idx].in_used = remain;
    }
}

void clients_feed_frames(Client* cs, int sender_idx, const unsigned char* data, size_t len, on_message_cb cb, void* user) {
    if (cs[sender_idx].fd == -1 ) {
        return;
    }

    if (len > INBUF_CAP - cs[sender_idx].in_used) {
        fprintf(stderr, "input buffer overflow from fd=%d. drop client.\n", cs[sender_idx].fd);
        clients_remove(cs, sender_idx);
        return;
    }

    memcpy(cs[sender_idx].inbuf + cs[sender_idx].in_used, data, len);
    cs[sender_idx].in_used += len;

    drain_frames_from_inbuf(cs, sender_idx, cb, user);
}

// sender의 name이 있으면 그걸 담고 length설정. 모든 클라이언트들에게 전송.
void clients_broadcast_text_framed(Client *cs, int sender_idx, const unsigned char *payload, size_t len) {
    char label[128];
    size_t llen = 0;

    if (cs[sender_idx].name[0] != '\0') {
        llen = (size_t)snprintf(label, sizeof(label), "[%s] ", cs[sender_idx].name);
    } else {
        llen = (size_t)snprintf(label, sizeof(label), "[%s:%u] ", cs[sender_idx].ip, cs[sender_idx].port);
    }
    if (llen >= sizeof(label)) llen = sizeof(label) - 1;

    unsigned char buf[INBUF_CAP];
    if (llen + len > sizeof(buf)) {
        fprintf(stderr, "broadcast message too large, skip.\n");
        return;
    }
    memcpy(buf, label, llen);
    memcpy(buf + llen, payload, len);

    uint32_t outlen = (uint32_t)(llen + len);

    for (int i=0; i< MAX_CLIENTS; i++) {
        int rfd = cs[i].fd;
        if (rfd == -1 || i == sender_idx) {
            continue;
        }
        if (send_frame(rfd, buf, outlen) < 0) {
            fprintf(stderr, "broadcast: send_frame failed → close %d\n", rfd);
            clients_remove(cs, i);
        }
    }
}

