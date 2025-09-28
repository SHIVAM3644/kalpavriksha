#include <stdio.h>
#include <ctype.h>
#include <string.h>

#define MAX_STACK_SIZE 1000


int calculate(const char s[]);

int main() {
    char expression[100];

    printf("Enter expression: ");
    scanf("%s", expression);   

    int result = calculate(expression);
    printf("Result: %d\n", result);

    return 0;
}

int calculate(const char s[]) {
    int stack[MAX_STACK_SIZE];
    int top = -1;

    int currentNumber = 0;
    char operation = '+';
    int len = strlen(s);

    for (int i = 0; i < len; i++) {
        char currentChar = s[i];

        if (isdigit(currentChar)) {
            currentNumber = currentNumber * 10 + (currentChar - '0');
        }

      
        if ((!isdigit(currentChar) && !isspace(currentChar)) || i == len - 1) {
            if (operation == '+') {
                stack[++top] = currentNumber;
            } else if (operation == '-') {
                stack[++top] = -currentNumber;
            } else if (operation == '*') {
                stack[top] = stack[top] * currentNumber;
            } else if (operation == '/') {
                stack[top] = stack[top] / currentNumber;
            }
            operation = currentChar;
            currentNumber = 0;
        }
    }

    int result = 0;
    for (int i = 0; i <= top; i++) {
        result += stack[i];
    }

    return result;
}
