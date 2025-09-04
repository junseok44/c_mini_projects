// clients.h
#pragma once
#include <netinet/in.h>
#include <stddef.h>
#include <sys/select.h>

#ifndef FD_SETSIZE
#define FD_SETSIZE 1024
#endif

#define MAX_CLIENTS (FD_SETSIZE - 1)

typedef struct {
    int fd;                         // -1이면 빈 슬롯
    struct sockaddr_in addr;        // 피어 주소
    char ip[64];                    // 사람이 보기 쉬운 IP 문자열
    unsigned short port;            // 호스트 바이트 오더
} Client;

// 초기화(모든 슬롯 fd=-1)
void clients_init(Client* cs);

// accept 후 신규 클라 등록, 성공 시 인덱스, 풀이면 -1
int clients_add(Client* cs, int cfd, struct sockaddr_in* cli);

// 클라 제거(close + fd=-1)
void clients_remove(Client* cs, int idx);

// fd로 인덱스 찾기(없으면 -1)
int clients_find_by_fd(Client* cs, int fd);

// select()용 읽기집합 구성
void clients_build_readfds(Client* cs, int lfd, fd_set* rfds);

// 현재 등록된 fd들의 최대값 반환(select의 maxfd 계산용)
int clients_max_fd(Client* cs, int lfd);

// sender_fd를 제외한 모든 대상에게 헤더 + payload 브로드캐스트
void clients_broadcast(Client* cs, int sender_fd, const char* data, size_t len);
