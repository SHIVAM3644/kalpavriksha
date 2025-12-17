#include <stdio.h>

#define MAXIMUM_STRING_LENGTH 100

int isWhitespace(char character)
{
    return (character == ' ' || character == '\t' || character == '\n');
}

int validateString(const char *string)
{
    int stringIndex = 0;
    int hasCharacter = 0;

    if (*(string + stringIndex) == '\n' || *(string + stringIndex) == '\0')
    {
        printf("Error: Please enter at least one character.\n");
        return 0;
    }

    while (*(string + stringIndex) != '\0')
    {
        if (!isWhitespace(*(string + stringIndex)))
        {
            hasCharacter = 1;
            break;
        }
        
        stringIndex++;
    }

    if (!hasCharacter)
    {
        printf("Error: Please enter at least one non-space character.\n");
        return 0;
    }

    return 1;
}

int main()
{
    char InputString[MAXIMUM_STRING_LENGTH];
    char newString[MAXIMUM_STRING_LENGTH];
    int stringIndex = 0;

    printf("Source: ");
    fgets(InputString, MAXIMUM_STRING_LENGTH, stdin);

    if (!validateString(InputString))
    {
        return 0;
    }

    while (*(InputString + stringIndex) != '\0')
    {
        *(newString + stringIndex) = *(InputString + stringIndex);
        stringIndex++;
    }

    *(newString + stringIndex) = '\0';

    printf("Copied string: %s", newString);

    return 0;
}
