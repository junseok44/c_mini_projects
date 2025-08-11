#include "number_stack.h"

void init2(NumberStack* stack) {
    stack -> top = -1;
}

int is_empty2(NumberStack* stack) {
    return stack -> top == -1;
}

void push2(NumberStack *stack, int value) {
    stack -> data[++(stack -> top)] = value;
}

int pop2(NumberStack *stack) {
    return stack -> data[(stack -> top)--];
}

int peek2(NumberStack *stack) {
    return stack -> data[stack -> top];
}