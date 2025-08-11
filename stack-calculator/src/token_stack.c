
#include "token_stack.h"

void init(TokenStack *stack) {
    stack->top = -1;
}

int is_empty(TokenStack *stack) {
    return stack->top == -1;
}

void push(TokenStack *stack, char* value) {
    stack -> data[++(stack->top)] = value;
}

char* pop(TokenStack *stack) {
    return stack -> data[(stack->top)--];
}

char* peek(TokenStack *stack) {
    return stack -> data[(stack->top)];
}