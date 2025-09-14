#pragma once
#include "common.h"

/* 
 * 한 줄을 토크나이즈하여 execvp용 argv 벡터를 만든다.
 * 정책(1주차):
 *  - 공백 분리
 *  - 작은따옴표('...')/큰따옴표("...")로 공백 포함 허용
 *  - 백슬래시(\) 이스케이프
 *  - 리다이렉션/파이프/백그라운드는 다음 주차에
 * 성공 시 out에 NULL-terminated argv 벡터가 들어감. (len >= 1 이면 마지막은 NULL)
 */
int tokenize(const char *line, StrVec *out);

typedef struct {
    char **argv;      // NULL-terminated (tokenize가 만든 포인터를 참조)
    char *in_path;    // NULL or path for '<'
    char *out_path;   // NULL or path for '>'
} SimpleCmd;

// 단일 파이프(|)와 리다이렉션(<, >)만 해석
int parse_simple(char **tokens, SimpleCmd *left, SimpleCmd *right, bool *has_pipe);

// 실행기
int run_simple(SimpleCmd *cmd);                      // 리다이렉션 적용 + 단일 명령 실행
int run_pipeline(SimpleCmd *left, SimpleCmd *right); // left | right

void free_simple_cmd(SimpleCmd *cmd);