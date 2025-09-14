#include "parse.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * 필요 전제:
 * - StrVec 유틸: sv_init / sv_push / sv_free_all
 * - StrVec 구조체, 그리고 이 유틸들은 기존 코드에 이미 있다고 전제
 *
 * 이번 버전에서 추가된 점:
 * - 파이프/리다이렉션 연산자 `|`, `<`, `>`를 단독 토큰으로 분리
 *   예) echo hi>out |cat  -> ["echo","hi",">","out","|","cat", NULL]
 * - 인용부(S_SQ, S_DQ) 내부에서는 연산자를 일반 문자로 취급
 */

int tokenize(const char *line, StrVec *out) {
    // 상태 머신: 바깥(S_OUT), 일반 단어(S_WORD), 작은따옴표(S_SQ), 큰따옴표(S_DQ)
    enum { S_OUT, S_WORD, S_SQ, S_DQ } st = S_OUT;

    StrVec tokens; 
    sv_init(&tokens);

    size_t cap = 64, len = 0;
    char *buf = (char *)malloc(cap);
    if (!buf) { perror("malloc"); return -1; }

    #define BUF_PUSH(ch) do { \
        if (len + 1 >= cap) { \
            size_t ncap = cap * 2; \
            char *nb = (char *)realloc(buf, ncap); \
            if (!nb) { \
                perror("realloc"); \
                free(buf); \
                sv_free_all(&tokens); \
                return -1; \
            } \
            buf = nb; \
            cap = ncap; \
        } \
        buf[len++] = (ch); \
    } while (0)

    for (size_t i = 0;; i++) {
        char c = line[i];
        int end = (c == '\0');

        if (st == S_OUT) {
            if (end) break;
            if (c==' '||c=='\t'||c=='\n') continue;

            // 연산자 단독 토큰 (따옴표 밖에서만)
            if (c=='|' || c=='<' || c=='>') {
                char op[2] = { c, '\0' };
                sv_push(&tokens, strdup(op));
                continue;
            }

            if (c=='\'') { st = S_SQ; continue; }
            if (c=='"')  { st = S_DQ; continue; }

            if (c=='\\') {
                if (line[i+1]) { BUF_PUSH(line[++i]); st = S_WORD; }
                else { /* 끝 직전의 '\'는 무시 */ }
                continue;
            }

            // 일반 문자 시작 → 단어 상태
            BUF_PUSH(c);
            st = S_WORD;
            continue;
        }

        if (st == S_WORD) {
            // 공백/개행/끝 → 단어 종료
            if (end || c==' '||c=='\t'||c=='\n') {
                BUF_PUSH('\0');
                sv_push(&tokens, strdup(buf));
                len = 0;
                st = S_OUT;
                if (end) break;
                continue;
            }

            // 단어 중 연산자 만나면: 현재 단어를 종료 후, 연산자 토큰 추가
            if (c=='|' || c=='<' || c=='>') {
                BUF_PUSH('\0');
                sv_push(&tokens, strdup(buf));
                len = 0;
                st = S_OUT;

                char op[2] = { c, '\0' };
                sv_push(&tokens, strdup(op));
                continue;
            }

            // 이스케이프
            if (c=='\\') {
                if (line[i+1]) BUF_PUSH(line[++i]);
                else { /* 끝 직전의 '\'는 무시 */ }
                continue;
            }

            // 인용 진입
            if (c=='\'') { st = S_SQ; continue; }
            if (c=='"')  { st = S_DQ; continue; }

            // 일반 문자
            BUF_PUSH(c);
            continue;
        }

        if (st == S_SQ) { // 작은따옴표: 이스케이프 없음, ' 로만 닫힘
            if (end) {
                fprintf(stderr, "syntax: unmatched '\n");
                sv_free_all(&tokens);
                free(buf);
                return -1;
            }
            if (c=='\'') { st = S_WORD; continue; }
            BUF_PUSH(c);
            continue;
        }

        if (st == S_DQ) { // 큰따옴표: \n, \t, \", \\만 특수 처리
            if (end) {
                fprintf(stderr, "syntax: unmatched \"\n");
                sv_free_all(&tokens);
                free(buf);
                return -1;
            }
            if (c=='"') { st = S_WORD; continue; }
            if (c=='\\') {
                char nx = line[i+1];
                if (!nx) continue; // 끝 직전 '\'
                switch (nx) {
                    case 'n':  BUF_PUSH('\n'); break;
                    case 't':  BUF_PUSH('\t'); break;
                    case '\\': BUF_PUSH('\\'); break;
                    case '"':  BUF_PUSH('"');  break;
                    default:   BUF_PUSH(nx);   break;
                }
                i++; 
                continue;
            }
            BUF_PUSH(c);
            continue;
        }
    }

    free(buf);

    // 빈 입력 → 빈 벡터로
    if (tokens.len == 0) {
        out->items = NULL;
        out->len = out->cap = 0;
        return 0;
    }

    // execvp 호출 편의를 위한 NULL 종단
    sv_push(&tokens, NULL);

    *out = tokens;
    return 0;

    #undef BUF_PUSH
}
