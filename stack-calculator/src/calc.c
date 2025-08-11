#include <ctype.h>
#include <stdatomic.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "number_stack.h"
#include "token_stack.h"    

#ifndef STACK_MAX
#define STACK_MAX 100
#endif

// ===== 우선순위 함수 =====
int precedence(const char* op) {
    if (strcmp(op, "+") == 0 || strcmp(op, "-") == 0) return 1;
    if (strcmp(op, "/") == 0 || strcmp(op, "*") == 0) return 2;
    return 0;
}

int is_number(char *token) {
    if (token == NULL || *token == '\0') return 0;

    if (*token == '-') token++;

    while (*token) {
        if (!isdigit(*token)) return 0;
        token++;
    }

    return 1;
}

// ===== 중위 → 후위 =====

// 연산자중에서 뭐가 우선적으로 계산되어야 해?
// +-는 나중에, 그러니까 stack 깊숙이 처박아 두고, * / 는 우선
// 그런데 같은 우선순위라면, 나중에 들어온거가 나중에 처리되어야 함.

// *************근데 이걸 스택으로밖에 구현을 못하나?*************

char** infix_to_postfix(char** tokens, int token_count, int* out_count) {
    TokenStack op_stack;
    init(&op_stack);

    char **output_tokens = malloc(sizeof(char *) * token_count);
    int token_idx = 0;
    
    for (int i=0; i<token_count; i++) {
        if (is_number(tokens[i])) {
            output_tokens[token_idx++] = tokens[i];
        } 
        // 이러면, 일단 스택에 넣는다.
        else if (strcmp(tokens[i],"(") == 0) {
            push(&op_stack, tokens[i]);
        }
        // 이러면, 지금까지 연산자 스택에 있던 것들을 다 꺼낸 다음, 
        // output 토큰에 더해주고, (는 빼준다.
        else if (strcmp(tokens[i],")") == 0) {
            while (strcmp(peek(&op_stack), "(") != 0) {
                output_tokens[token_idx++] = pop(&op_stack);
            }
            pop(&op_stack);
        }
        else {
            // 지금 top에 있는 연산자의 우선 순위가 높거나 같다? 그러면 그건 먼저 계산해야 하는거니까 꺼낸다.
            while (!is_empty(&op_stack) && precedence(peek(&op_stack)) >= precedence(tokens[i])) {
                output_tokens[token_idx++] = pop(&op_stack);
            }
            push(&op_stack, tokens[i]);
        }
    }

    while (!is_empty(&op_stack)) {
        output_tokens[token_idx++] = pop(&op_stack);
    }

    *out_count = token_idx;

    return output_tokens;
}

int calculate_postfix_tokens(char** tokens, int token_count) {
    NumberStack numberStack;
    init2(&numberStack);

    for(int i=0; i<token_count; i++) {
        char *target = tokens[i];
        if (is_number(target)) {
            push2(&numberStack, atoi(target));
        } else if (strcmp(target, "+") == 0 || strcmp(target, "-") == 0 || strcmp(target, "*") == 0 || strcmp(target, "/") == 0) {
            // 이러면 스택에서 숫자 두개 꺼내서 연산.
            int s2 = pop2(&numberStack);
            int s1 = pop2(&numberStack);

            if (strcmp(target,"+") == 0) {
                push2(&numberStack, s1+s2);
            } else if (strcmp(target, "-") == 0) {
                push2(&numberStack, s1-s2);
            } else if (strcmp(target, "*") == 0) {
                push2(&numberStack, s1*s2);
            } else if (strcmp(target, "/") == 0) {
                push2(&numberStack, s1/s2);
            } 
        }
    }   

    return pop2(&numberStack);
}

int calc(char *expr) {
    const char* delim = " ";
    char **tokens = malloc(sizeof(char *) * STACK_MAX);
    char *token = strtok(expr, delim);
    int token_count = 0;
    int out_count = 0;

    while (token != NULL && token_count < STACK_MAX) {
        tokens[token_count] = malloc(strlen(token)+1);
        strcpy(tokens[token_count], token);
        printf("토큰 %d : %s\n", token_count, tokens[token_count]);
        token_count++;
        token = strtok(NULL,delim);
    }

    char** postfix_tokens = infix_to_postfix(tokens, token_count, &out_count);

    int result = calculate_postfix_tokens(postfix_tokens, out_count);

    for(int i=0; i < STACK_MAX; i++) {
        free(tokens[i]);
    }
    free(tokens);
    free(postfix_tokens);

    return result;
}