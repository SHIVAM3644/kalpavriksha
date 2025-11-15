#include <stdio.h>

int main() 
{
    int elementSize;
    int alternateElementSum = 0;

    printf("Enter number of elements: ");
    
    if (scanf("%d", &elementSize) != 1 || elementSize <= 0) 
    {
        printf("Error: Please enter a valid positive integer for number of elements.\n");
        return 1;
    }

    int inputArray[elementSize];

    printf("Enter elements: ");
    for (int arrayIndex = 0; arrayIndex < elementSize; arrayIndex++) 
    {
        if (scanf("%d", &inputArray[arrayIndex]) != 1) 
        {
            printf("Error: Invalid input detected. Please enter integers only.\n");
            return 1;
        }
    }

    for (int arrayIndex = 0; arrayIndex < elementSize; arrayIndex += 2) 
    {
        alternateElementSum += inputArray[arrayIndex];
    }

    printf("Sum of alternate elements = %d\n", alternateElementSum);

    return 0;
}
