#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void printMatrix(int *pointerToArray, int dimensionOfMatrix);
int validateInput(int noOfDimension);
void getRotatedMatrix(int *pointerToArray, int dimensionOfMatrix);
void getSmoothingMatrix(int *pointerToArray, int dimensionOfMatrix);
void swapDigits(int *firstValue, int *secondValue);

int main(void)
{
    srand(time(0));
    int noOfDimension;

    printf("Please Enter dimension of matrix (2-10): \n");
    
    if (scanf("%d", &noOfDimension) != 1)
    {
        printf("Invalid input. Please enter a number between 2 and 10.\n");
        return 1;
    }

    int matrix[noOfDimension][noOfDimension];
    int *pointerToArray = (int *)matrix;

    if (validateInput(noOfDimension) == 0)
    {
        printf("Please enter a valid dimension between 2 and 10.\n");
    }
    else
    {
        printf("\nOriginal:\n");

        for (int rowIndex = 0; rowIndex < noOfDimension; rowIndex++)
        {
            for (int columnIndex = 0; columnIndex < noOfDimension; columnIndex++)
            {
                *(pointerToArray + rowIndex * noOfDimension + columnIndex) = rand() % 256;
            }
        }

        printMatrix(pointerToArray, noOfDimension);

        printf("\nRotated:\n");
        getRotatedMatrix(pointerToArray, noOfDimension);

        printf("\nFinal Output:\n");
        getSmoothingMatrix(pointerToArray, noOfDimension);
    }

    return 0;
}

void printMatrix(int *pointerToArray, int dimensionOfMatrix)
{
    for (int rowIndex = 0; rowIndex < dimensionOfMatrix; rowIndex++)
    {
        for (int columnIndex = 0; columnIndex < dimensionOfMatrix; columnIndex++)
        {
            printf("%d ", *(pointerToArray + rowIndex * dimensionOfMatrix + columnIndex));
        }
        printf("\n");
    }
}

int validateInput(int noOfDimension)
{
    return noOfDimension >= 2 && noOfDimension <= 10;
}

void getRotatedMatrix(int *pointerToArray, int dimensionOfMatrix)
{
    for (int rowIndex = 0; rowIndex < dimensionOfMatrix - 1; rowIndex++)
    {
        for (int columnIndex = rowIndex + 1; columnIndex < dimensionOfMatrix; columnIndex++)
        {
            swapDigits(pointerToArray + rowIndex * dimensionOfMatrix + columnIndex,
                        pointerToArray + columnIndex * dimensionOfMatrix + rowIndex);
        }
    }

    for (int rowIndex = 0; rowIndex < dimensionOfMatrix; rowIndex++)
    {
        for (int columnIndex = 0; columnIndex < dimensionOfMatrix / 2; columnIndex++)
        {
            swapDigits(pointerToArray + rowIndex * dimensionOfMatrix + columnIndex,
                        pointerToArray + rowIndex * dimensionOfMatrix + (dimensionOfMatrix - 1 - columnIndex));
        }
    }

    printMatrix(pointerToArray, dimensionOfMatrix);
}

void getSmoothingMatrix(int *pointerToArray, int dimensionOfMatrix)
{
    int previousRowSmoothValues[10];
    int currentRowSmoothValues[10];
    int canWritePreviousRow = 0;

    for (int rowIndex = 0; rowIndex < dimensionOfMatrix; rowIndex++)
    {
        for (int columnIndex = 0; columnIndex < dimensionOfMatrix; columnIndex++)
        {
            int sumOfNeighbours = 0;
            int validNeighbourCount = 0;
            int startRowIndex = (rowIndex > 0) ? rowIndex - 1 : rowIndex;
            int endRowIndex = (rowIndex < dimensionOfMatrix - 1) ? rowIndex + 1 : rowIndex;
            int startColumnIndex = (columnIndex > 0) ? columnIndex - 1 : columnIndex;
            int endColumnIndex = (columnIndex < dimensionOfMatrix - 1) ? columnIndex + 1 : columnIndex;

            int *neighbourPointer = pointerToArray + startRowIndex * dimensionOfMatrix + startColumnIndex;

            for (int neighbourRowIndex = startRowIndex; neighbourRowIndex <= endRowIndex; neighbourRowIndex++)
            {
                int windowWidth = endColumnIndex - startColumnIndex + 1;

                for (int neighbourColumnOffset = 0; neighbourColumnOffset < windowWidth; neighbourColumnOffset++)
                {
                    sumOfNeighbours += *(neighbourPointer + neighbourColumnOffset);
                    validNeighbourCount++;
                }

                neighbourPointer += dimensionOfMatrix;
            }

            *(currentRowSmoothValues + columnIndex) = sumOfNeighbours / validNeighbourCount;
        }

        if (canWritePreviousRow)
        {
            int *matrixWritePointer = pointerToArray + (rowIndex - 1) * dimensionOfMatrix;

            for (int columnOffset = 0; columnOffset < dimensionOfMatrix; columnOffset++)
            {
                *(matrixWritePointer + columnOffset) = *(previousRowSmoothValues + columnOffset);
            }
        }

        for (int columnOffset = 0; columnOffset < dimensionOfMatrix; columnOffset++)
        {
            *(previousRowSmoothValues + columnOffset) = *(currentRowSmoothValues + columnOffset);
        }

        canWritePreviousRow = 1;
    }

    if (canWritePreviousRow)
    {
        int *matrixWritePointer = pointerToArray + (dimensionOfMatrix - 1) * dimensionOfMatrix;
        
        for (int columnOffset = 0; columnOffset < dimensionOfMatrix; columnOffset++)
        {
            *(matrixWritePointer + columnOffset) = *(previousRowSmoothValues + columnOffset);
        }
    }

    printMatrix(pointerToArray, dimensionOfMatrix);
}

void swapDigits(int *firstValue, int *secondValue)
{
    int temporaryVariable = *firstValue;
    *firstValue = *secondValue;
    *secondValue = temporaryVariable;
}
