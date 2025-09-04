// net.h
#pragma once
#include <stdint.h>
#include <stddef.h>

// 리스닝 소켓 생성(AF_INET, 0.0.0.0:port, SO_REUSEADDR, listen)
int setup_listen_socket(uint16_t port);

// send() 부분 전송을 고려하여 len 바이트를 끝까지 보냄
// 성공 0, 실패 -1(EPIPE/ECONNRESET 등) 반환
int send_all(int fd, const void* buf, size_t len);