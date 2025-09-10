#include "parse.h"

int tokenize(const char *line, StrVec *out) {
    // 상태 머신: 바깥(S_OUT), 일반 단어(S_WORD), 작은따옴표(S_SQ), 큰따옴표(S_DQ)
    enum { S_OUT, S_WORD, S_SQ, S_DQ } st = S_OUT;

    // 토큰 동적 배열 준비 (sv_init/sv_push/sv_free_all은 StrVec 유틸이라 가정)
    StrVec tokens; sv_init(&tokens);

    // 현재 단어를 임시로 쌓을 작업 버퍼
    size_t cap = 64, len = 0;
    char *buf = malloc(cap);
    if (!buf) { perror("malloc"); return -1; }

    // 안전한 push 매크로: cap 모자라면 2배로 늘림 (realloc 실패 시 즉시 정리/리턴)
    #define BUF_PUSH(ch) do { \
        if (len + 1 >= cap) { \
            cap *= 2; \
            char *nb = realloc(buf, cap); \
            if (!nb) { \
                perror("realloc"); \
                free(buf); \
                sv_free_all(&tokens); \
                return -1; \
            } \
            buf = nb; \
        } \
        buf[len++] = (ch); \
    } while(0)

    // 한 글자씩 읽으면서 상태 머신으로 파싱
    for (size_t i = 0;; i++) {
        char c = line[i];
        int end = (c == '\0');      // 문자열 끝 여부

        if (st == S_OUT) {          // 단어 바깥: 공백을 건너뛰고, 단어 시작을 찾는다
            if (end) break;         // 입력 끝
            if (c==' '||c=='\t'||c=='\n') continue;   // 공백류 무시

            if (c=='\'') { st = S_SQ; continue; }     // 작은따옴표 구간 진입
            if (c=='"')  { st = S_DQ; continue; }     // 큰따옴표 구간 진입

            if (c=='\\') {                             // 바깥에서의 백슬래시: 다음 글자 리터럴화
                if (line[i+1]) { BUF_PUSH(line[++i]); st = S_WORD; }
                continue;
            }

            // 일반 문자면 단어 시작
            BUF_PUSH(c);
            st = S_WORD;
            continue;
        }

        if (st == S_WORD) {         // 일반 단어 구간
            // 공백/개행/끝을 만나면 현재 단어 종료
            if (end || c==' '||c=='\t'||c=='\n') {
                BUF_PUSH('\0');                       // C 문자열 종단
                sv_push(&tokens, strdup(buf));        // ★ 복사본을 토큰 배열에 저장
                len = 0;                              // 작업 버퍼 리셋(다음 토큰용)
                st = S_OUT;                           // 바깥 상태로 복귀
                if (end) break;                       // 끝이면 루프 종료
                continue;
            }

            // 백슬래시: 다음 글자를 그대로 추가(이스케이프)
            if (c=='\\') {
                if (line[i+1]) BUF_PUSH(line[++i]);
                continue;
            }

            // 인용부를 단어 중간에 섞을 수 있음
            if (c=='\'') { st = S_SQ; continue; }     // 작은따옴표 내부로
            if (c=='"')  { st = S_DQ; continue; }     // 큰따옴표 내부로

            // 그 외 일반 문자
            BUF_PUSH(c);
            continue;
        }

        if (st == S_SQ) {           // 작은따옴표 내부: 이스케이프 없음, ' 로만 닫힘
            if (end) {              // 닫히지 않고 끝남 → 문법 오류
                fprintf(stderr, "syntax: unmatched '\n");
                sv_free_all(&tokens);
                free(buf);
                return -1;
            }
            if (c=='\'') {          // 닫힘 → 일반 단어 상태로 복귀
                st = S_WORD;
                continue;
            }
            BUF_PUSH(c);            // 그대로 추가
            continue;
        }

        if (st == S_DQ) {           // 큰따옴표 내부: 일부 이스케이프 지원
            if (end) {              // 닫히지 않고 끝남 → 문법 오류
                fprintf(stderr, "syntax: unmatched \"\n");
                sv_free_all(&tokens);
                free(buf);
                return -1;
            }
            if (c=='"') {           // 닫힘 → 일반 단어 상태로 복귀
                st = S_WORD;
                continue;
            }
            if (c=='\\') {          // 이스케이프 처리: \n, \t, \", 
                char nx = line[i+1];
                if (!nx) continue;  // 끝 직전의 '\'는 무시
                switch (nx) {
                    case 'n':  BUF_PUSH('\n'); break;
                    case 't':  BUF_PUSH('\t'); break;
                    case '\\': BUF_PUSH('\\'); break;
                    case '"':  BUF_PUSH('"');  break;
                    default:   BUF_PUSH(nx);   break; // 그 외는 그대로
                }
                i++; continue;      // 다음 글자를 소비했으니 인덱스 추가
            }
            BUF_PUSH(c);            // 일반 문자
            continue;
        }
    }

    // 작업 버퍼는 더 이상 필요 없음
    free(buf);

    // 토큰이 하나도 없으면 깔끔한 빈 벡터로 반환
    if (tokens.len == 0) {
        out->items = NULL;
        out->len = out->cap = 0;
        return 0;
    }

    // execvp(argv[0], argv) 형태로 바로 쓰기 위한 NULL 종단
    sv_push(&tokens, NULL);

    // 호출자에게 소유권 이전
    *out = tokens;
    return 0;
}
