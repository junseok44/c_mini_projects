#ifndef NUMBER_STACK_H
#define NUMBER_STACK_H

#define STACK_MAX_NUMBER 100

typedef struct {
    int data[STACK_MAX_NUMBER];
    int top;
} NumberStack;

void init2(NumberStack *stack);

int is_empty2(NumberStack *stack);

void push2(NumberStack *stack, int value);

int pop2(NumberStack *stack);

int peek2(NumberStack *stack);

#endif