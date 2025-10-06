#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>

#define MAX_STACK_SIZE 1000

void calculate(const char s[]);
bool isValidExpression(const char s[]);

int main()
{
    char expression[100];

    fgets(expression, sizeof(expression), stdin);

    if (!isValidExpression(expression))
    {
        printf("Error: Invalid expression.\n");
        return 0;
    }

    calculate(expression);

    return 0;
}

void calculate(const char s[])
{
    int stack[MAX_STACK_SIZE];
    int top = -1;
    bool isDivisibleByZero = false;
    int currentNumber = 0;
    char operation = '+';
    int len = strlen(s);

    for (int i = 0; i < len; i++)
    {
        char currentChar = s[i];
        if (currentChar == ' ')
        {
            continue;
        }
        if (isdigit((unsigned char)currentChar))
        {
            currentNumber = currentNumber * 10 + (currentChar - '0');
        }

        if (!isdigit((unsigned char)currentChar) || i == len - 1)
        {
            if (operation == '+')
            {
                stack[++top] = currentNumber;
            }
            else if (operation == '-')
            {
                stack[++top] = -currentNumber;
            }
            else if (operation == '*')
            {
                stack[top] = stack[top] * currentNumber;
            }
            else if (operation == '/')
            {
                if (currentNumber == 0)
                {
                    isDivisibleByZero = true;
                    break;
                }
                stack[top] = stack[top] / currentNumber;
            }

            operation = currentChar;
            currentNumber = 0;

        }
    }


    if (isDivisibleByZero)
    {
        printf("Error: Division by zero.");
    }
    else
    {
        int result = 0;
        for (int i = 0; i <= top; i++)
        {
            result += stack[i];
        }
        printf("Result of Expression : %d\n", result);
    }
}


bool isValidExpression(const char s[])

{
    for (int i = 0; s[i] != '\0'; i++)

    {

        if (s[i] == '\n')
           {
             continue;
           }
        if (s[i] == ' ')
           {
             continue;
           }
        if (!isdigit((unsigned char)s[i]) && s[i] != '+' && s[i] != '-' && s[i] != '*' && s[i] != '/' && s[i] != ' ')
        {
            return false;
        }
    }
    return true;
}
