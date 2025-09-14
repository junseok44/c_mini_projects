#include "sig.h"
#include <signal.h>
#include <stdio.h>

/* 부모 셸은 Ctrl+C(SIGINT)를 무시 → 셸은 죽지 않음 */
void ignore_sigint_in_parent(void) {
    struct sigaction sa = {0};
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART; /* 블로킹 호출 재시작 */
    if (sigaction(SIGINT, &sa, NULL) < 0) perror("sigaction(SIGINT)");
}

/* 자식은 SIGINT 기본 동작 복원 → 실행 중 프로그램을 Ctrl+C로 중지 가능 */
void restore_default_sigint(void) {
    struct sigaction sa = {0};
    sa.sa_handler = SIG_DFL;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGINT, &sa, NULL) < 0) perror("sigaction(SIGINT child)");
}
