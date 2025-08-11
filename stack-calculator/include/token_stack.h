#ifndef STACK_TOKEN_H
#define STACK_TOKEN_H

#define STACK_MAX_TOKEN 100

typedef struct {
    char* data[STACK_MAX_TOKEN];
    int top;
} TokenStack;

void init(TokenStack *stack);

int is_empty(TokenStack *stack); 

void push(TokenStack *stack, char* value);

char* pop(TokenStack *stack);

char* peek(TokenStack *stack);

#endif