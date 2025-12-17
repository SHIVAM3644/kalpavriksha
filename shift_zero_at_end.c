#include <stdio.h>

void moveZerosToEnd(int *pointerToArray, int size) 
{
    int *nonZeroPtr = pointerToArray;       
    int *currentPtr = pointerToArray;       
    int *endPtr = pointerToArray + size;    

    while (currentPtr < endPtr) 
    {
        if (*currentPtr != 0) 
        {
            *nonZeroPtr = *currentPtr;
            nonZeroPtr++;
        }
        currentPtr++;
    }

    while (nonZeroPtr < endPtr) 
    {
        *nonZeroPtr = 0;
        nonZeroPtr++;
    }
}

int main() 
{
    int elementSize;

    printf("Enter number of elements: ");

    if (scanf("%d", &elementSize) != 1 || elementSize <= 0) 
    {
        printf("Error: Please enter a valid positive integer for number of elements.\n");
        return 1;
    }

    int inputArray[elementSize];
    int *pointerToArray = inputArray;

    printf("Enter array elements: ");

    for (int arrayIndex = 0; arrayIndex < elementSize; arrayIndex++, pointerToArray++) 
    {
        if (scanf("%d", pointerToArray) != 1) 
        {
            printf("Error: Invalid input detected. Please enter integers only.\n");
            return 1;
        }
    }

    moveZerosToEnd(inputArray, elementSize);

    printf("Array after moving zeros: ");
    pointerToArray = inputArray;

    for (int arrayIndex = 0; arrayIndex < elementSize; arrayIndex++, pointerToArray++) 
    {
        printf("%d ", *pointerToArray);
    }

    printf("\n");

    return 0;
}
