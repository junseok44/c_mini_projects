#ifndef COMMON_H
#define COMMON_H

#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

/* argv 동적 벡터 (execvp 인자용) */
typedef struct {
    char **items;
    size_t len;
    size_t cap;
} StrVec;

/* 기본 유틸 */
static inline void sv_init(StrVec *v) {
    v->items = NULL; v->len = 0; v->cap = 0;
}
static inline void sv_push(StrVec *v, char *s) {
    if (v->len + 1 > v->cap) {
        size_t ncap = v->cap ? v->cap * 2 : 8;
        char **ni = (char **)realloc(v->items, ncap * sizeof(char*));
        if (!ni) { perror("realloc"); exit(1); }
        v->items = ni; v->cap = ncap;
    }
    v->items[v->len++] = s;
}
static inline void sv_free_all(StrVec *v) {
    for (size_t i = 0; i + 1 < v->len; i++) free(v->items[i]); /* 마지막 NULL 제외 */
    free(v->items);
    v->items = NULL; v->len = v->cap = 0;
}

/* 좌우 공백 제거 */
static inline char* str_trim(char *s) {
    while (*s && (*s==' '||*s=='\t'||*s=='\n')) s++;
    size_t n = strlen(s);
    while (n>0 && (s[n-1]==' '||s[n-1]=='\t'||s[n-1]=='\n')) { s[--n] = '\0'; }
    return s;
}

#endif /* COMMON_H */
