#define _POSIX_C_SOURCE 200112L
#include "clients.h"
#include "net.h"

#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>

void clients_init(Client* cs) {
    for (int i = 0; i < MAX_CLIENTS; i++) cs[i].fd = -1;
}

int clients_add(Client* cs, int cfd, struct sockaddr_in* cli) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (cs[i].fd == -1) {
            cs[i].fd   = cfd;
            cs[i].addr = *cli;
            inet_ntop(AF_INET, &cli->sin_addr, cs[i].ip, sizeof(cs[i].ip));
            cs[i].port = (unsigned short)ntohs(cli->sin_port);
            return i;
        }
    }
    return -1;
}

void clients_remove(Client *cs, int idx) {
    if (cs[idx].fd != -1) {
        close(cs[idx].fd);
    }
}

int clients_find_by_fd(Client* cs, int fd) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (cs[i].fd == fd) return i;
    }
    return -1;
}

int clients_max_fd(Client* cs, int lfd) {
    int maxfd = lfd;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (cs[i].fd > maxfd) maxfd = cs[i].fd;
    }
    return maxfd;
}

void clients_broadcast(Client* cs, int sender_fd, const char* data, size_t len) {
    int sidx = clients_find_by_fd(cs, sender_fd);

    // 전송 시 간단한 프리픽스 헤더(송신자 표식)
    char header[128];
    size_t hlen;
    if (sidx >= 0) {
        hlen = (size_t)snprintf(header, sizeof(header), "[%s:%u] ",
                                cs[sidx].ip, cs[sidx].port);
        if (hlen >= sizeof(header)) hlen = sizeof(header) - 1;
    } else {
        hlen = (size_t)snprintf(header, sizeof(header), "[unknown] ");
        if (hlen >= sizeof(header)) hlen = sizeof(header) - 1;
    }

    for (int i = 0; i < MAX_CLIENTS; i++) {
        int dfd = cs[i].fd;
        if (dfd == -1 || dfd == sender_fd) continue;

        if (send_all(dfd, header, hlen) < 0 || send_all(dfd, data, len) < 0) {
            // 실패한 대상은 끊긴 것으로 간주하고 정리
            fprintf(stderr, "broadcast: send failed → close %d\n", dfd);
            clients_remove(cs, i);
        }
    }
}