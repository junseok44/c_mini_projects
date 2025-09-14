# Socket Server - Multi-client Chat Server

TCP 소켓 기반의 다중 클라이언트 채팅 서버입니다. select() 기반의 I/O 멀티플렉싱과 프레임 기반 프로토콜을 사용합니다.

## 기능

- **다중 클라이언트 지원**: 최대 1023개의 동시 연결 (FD_SETSIZE - 1)
- **실시간 채팅**: 클라이언트 간 실시간 메시지 교환
- **닉네임 기능**: `/nick` 명령어로 사용자 이름 설정
- **프레임 기반 프로토콜**: 길이 정보가 포함된 안정적인 메시지 전송
- **논블로킹 I/O**: select()를 사용한 효율적인 다중 연결 처리

## 빌드

```bash
make
```

## 사용법

### 서버 실행
```bash
./bin/socket-server <포트번호>
```

예시:
```bash
./bin/socket-server 12345
```

### 클라이언트 연결
telnet이나 netcat 등의 도구로 연결:
```bash
telnet localhost 12345
# 또는
nc localhost 12345
```

## 프로토콜

### 프레임 형식
모든 메시지는 다음 형식으로 전송됩니다:
```
[4바이트 길이(빅엔디언)] + [페이로드]
```

- **길이 헤더**: 네트워크 바이트 오더(빅엔디언)로 페이로드 크기
- **페이로드**: 실제 메시지 내용 (UTF-8 텍스트)

### 명령어

#### 닉네임 설정
```
/nick <사용자이름>
```
예시:
```
/nick Alice
```

#### 일반 메시지
닉네임 설정 후 모든 텍스트는 브로드캐스트됩니다:
```
Hello everyone!
```

## 서버 출력 예시

```bash
$ ./bin/socket-server 12345
[week3] listening on port 12345 (select + framed protocol)
client connected: 127.0.0.1:54321 (fd=4, slot=0)
client connected: 127.0.0.1:54322 (fd=5, slot=1)
client disconnected: 127.0.0.1:54321 (fd=4, slot=0)
```

## 클라이언트 경험

```bash
$ telnet localhost 12345
Trying 127.0.0.1...
Connected to localhost.
Escape character is '^]'.
[server] welcome! use '/nick yourname' to set nickname.

/nick Alice
[server] nick set to 'Alice'

Hello everyone!
[Alice] Hello everyone!

/nick Bob  
[server] nick set to 'Bob'

Nice to meet you Alice!
[Bob] Nice to meet you Alice!
```

## 프로젝트 구조

```
socket-server/
├── src/
│   ├── main.c          # 메인 서버 로직, select() 루프
│   ├── net.c           # 네트워크 유틸리티 함수
│   └── clients.c       # 클라이언트 관리 및 프레이밍
├── include/
│   ├── net.h           # 네트워크 함수 선언
│   └── clients.h       # 클라이언트 구조체 및 함수
└── bin/
    └── socket-server   # 빌드된 실행 파일
```

## 구현 특징

### I/O 멀티플렉싱 (select)
```c
fd_set rfds;
clients_build_readfds(clients, listen_fd, &rfds);
int ready = select(maxfd + 1, &rfds, NULL, NULL, NULL);
```
- **논블로킹**: 여러 소켓을 동시에 모니터링
- **효율성**: 이벤트 기반 처리로 CPU 효율적 사용
- **확장성**: 단일 스레드로 다중 클라이언트 처리

### 프레임 기반 프로토콜
```c
// 전송: [길이 4바이트] + [페이로드]
int send_frame(int fd, const void* payload, uint32_t len);

// 수신: 누적 버퍼에서 완성된 프레임 추출
void clients_feed_frames(Client* cs, int sender_idx, ...);
```
- **메시지 경계**: 각 메시지의 정확한 경계 구분
- **부분 수신 처리**: TCP의 스트림 특성을 고려한 누적 처리
- **무결성**: 메시지 손실이나 합체 방지

### 클라이언트 관리
```c
typedef struct {
    int fd;                         // 소켓 파일 디스크립터
    struct sockaddr_in addr;        // 클라이언트 주소
    char ip[64];                    // IP 문자열
    unsigned short port;            // 포트 번호
    char name[32];                  // 닉네임
    unsigned char inbuf[INBUF_CAP]; // 수신 버퍼
    size_t in_used;                 // 사용된 버퍼 크기
} Client;
```
- **배열 기반**: 고정 크기 배열로 간단한 관리
- **슬롯 재사용**: 연결 해제 시 슬롯 재사용
- **상태 추적**: 각 클라이언트의 연결 상태와 정보 유지

### 신호 처리
```c
signal(SIGPIPE, SIG_IGN);  // SIGPIPE 무시
```
- **SIGPIPE 무시**: 클라이언트 연결 해제 시 서버 종료 방지
- **안정성**: 예기치 않은 연결 해제에 대한 견고성

## 네트워크 함수

### setup_listen_socket()
- TCP 소켓 생성 및 바인딩
- SO_REUSEADDR 옵션 설정
- 지정된 포트에서 리스닝 시작

### send_all()
- 부분 전송을 고려한 완전 전송
- EINTR 신호 처리
- 연결 오류 감지

### send_frame()
- 프레임 헤더(길이) + 페이로드 전송
- 네트워크 바이트 오더 변환
- 원자적 전송 보장

## 성능 특징

- **동시 연결**: 최대 1023개 클라이언트 동시 처리
- **메모리 효율**: 고정 크기 구조체로 메모리 사용량 예측 가능
- **CPU 효율**: select() 기반 이벤트 드리븐 아키텍처
- **네트워크 효율**: 프레임 기반으로 불필요한 파싱 최소화

## 제한사항

- **최대 연결 수**: FD_SETSIZE에 의해 제한 (일반적으로 1024)
- **메시지 크기**: 최대 64KB 프레임 크기
- **단일 스레드**: CPU 집약적 작업 시 모든 클라이언트 블로킹
- **메모리 사용**: 모든 클라이언트 슬롯을 사전 할당

## 학습 내용

이 프로젝트를 통해 학습할 수 있는 네트워크 프로그래밍 개념:

- **TCP 소켓 프로그래밍**: socket, bind, listen, accept
- **I/O 멀티플렉싱**: select() 시스템 콜
- **비동기 I/O**: 논블로킹 소켓 처리
- **프로토콜 설계**: 프레임 기반 메시징
- **클라이언트 관리**: 연결 상태 추적
- **신호 처리**: SIGPIPE 등 네트워크 관련 신호
- **바이트 오더**: 네트워크 바이트 오더 변환
- **오류 처리**: 네트워크 오류 상황 대응
