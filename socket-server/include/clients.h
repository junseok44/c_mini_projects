// clients.h
#pragma once
#include <netinet/in.h>
#include <stddef.h>
#include <sys/select.h>

#define MAX_CLIENTS   (FD_SETSIZE - 1)

// 1프레임 최대 크기(실습용). 너무 크게 두지 말기.
#define MAX_FRAME_SIZE (64 * 1024)

// 수신 누적 버퍼 크기: 길이(4) + 본문 최대
#define INBUF_CAP (4 + MAX_FRAME_SIZE)

typedef struct {
    int fd;                         // -1이면 빈 슬롯
    struct sockaddr_in addr;        // 피어 주소
    char ip[64];                    // 사람이 보기 쉬운 IP 문자열
    unsigned short port;            // 호스트 바이트 오더

    // --- 3주차: 닉네임 & 수신 누적 버퍼 ---
    char name[32];                  // /nick 으로 설정 (기본은 빈문자열)
    unsigned char inbuf[INBUF_CAP]; // 수신 누적 버퍼
    size_t in_used;                 // inbuf에 현재 쌓인 바이트 수
} Client;

// 초기화(모든 슬롯 비우기)
void clients_init(Client* cs);

// accept 후 신규 클라 등록, 성공 시 인덱스, 풀이면 -1
int clients_add(Client* cs, int cfd, struct sockaddr_in* cli);

// 클라 제거(close + fd=-1로 비우기)
void clients_remove(Client* cs, int idx);

// fd로 인덱스 찾기(없으면 -1)
int clients_find_by_fd(Client* cs, int fd);

// select()용 읽기집합 구성
void clients_build_readfds(Client* cs, int lfd, fd_set* rfds);

// 현재 등록된 fd들의 최대값 반환(select의 maxfd 계산용)
int clients_max_fd(Client* cs, int lfd);

// [프레이밍 수신] data를 inbuf에 누적하고, 완성된 프레임마다 on_message 콜백 호출
// on_message(cs, sender_idx, payload, len, user_data)
typedef void (*on_message_cb)(Client* cs, int sender_idx, const unsigned char* payload, size_t len, void* user);

void clients_feed_frames(Client* cs, int sender_idx,
                         const unsigned char* data, size_t len,
                         on_message_cb cb, void* user);

// [프레이밍 브로드캐스트] 송신자 라벨을 프리픽스로 붙여 프레임으로 전송
void clients_broadcast_text_framed(Client* cs, int sender_idx,
                                   const unsigned char* payload, size_t len);
