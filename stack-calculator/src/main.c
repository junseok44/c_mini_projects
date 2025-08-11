#include <stdio.h>
#include "calc.h"
#include <string.h>

int main() {
    char exp[100];
    printf("enter the expression: ");
    fgets(exp,100,stdin);
    exp[strcspn(exp, "\n")] = '\0';

    int result = calc(exp);

    printf("result: %d\n", result);

    return 0;
}