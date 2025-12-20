#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_ELEMENTS 50
#define DATA_FILE "ipc_data.txt"

void terminateWithError(const char *errorMessage)
{
    fprintf(stderr, "ERROR: %s\n", errorMessage);
    exit(EXIT_FAILURE);
}

void validateInputCount(int expectedCount, int actualCount)
{
    if (expectedCount != actualCount)
    {
        terminateWithError("Invalid number of elements provided");
    }
}

void sortArrayAscending(int *numberArray, int elementCount)
{
    int firstIndex;
    int secondIndex;
    int temporaryValue;

    for (firstIndex = 0; firstIndex < elementCount - 1; firstIndex++)
    {
        for (secondIndex = firstIndex + 1; secondIndex < elementCount; secondIndex++)
        {
            if (numberArray[firstIndex] > numberArray[secondIndex])
            {
                temporaryValue = numberArray[firstIndex];
                numberArray[firstIndex] = numberArray[secondIndex];
                numberArray[secondIndex] = temporaryValue;
            }
        }
    }
}

int main(void)
{
    int elementCount;
    int numberArray[MAX_ELEMENTS];
    pid_t childProcessId;

    printf("Enter number of elements: ");

    if (scanf("%d", &elementCount) != 1 || elementCount <= 0 || elementCount > MAX_ELEMENTS)
    {
        terminateWithError("Invalid element count");
    }

    printf("Enter %d integers:\n", elementCount);

    int readCounter = 0;

    while (readCounter < elementCount)
    {
        if (scanf("%d", &numberArray[readCounter]) != 1)
        {
            terminateWithError("Invalid integer input");
        }

        readCounter++;
    }

    int extraCharacter;
    extraCharacter = getchar();

    if (extraCharacter != '\n' && extraCharacter != EOF)
    {
        terminateWithError("Extra input detected");
    }


    FILE *fileWriter = fopen(DATA_FILE, "w");

    if (fileWriter == NULL)
    {
        terminateWithError("Failed to open file for writing");
    }

    fprintf(fileWriter, "%d\n", elementCount);

    int writeIndex = 0;

    while (writeIndex < elementCount)
    {
        fprintf(fileWriter, "%d ", numberArray[writeIndex]);
        writeIndex++;
    }

    fclose(fileWriter);

    printf("Array before sorting:\n");

    int displayIndex = 0;

    while (displayIndex < elementCount)
    {
        printf("%d ", numberArray[displayIndex]);
        displayIndex++;
    }
    printf("\n");

    childProcessId = fork();

    if (childProcessId < 0)
    {
        terminateWithError("Fork failed");
    }

    if (childProcessId == 0)
    {
        FILE *fileReader = fopen(DATA_FILE, "r");

        if (fileReader == NULL)
        {
            terminateWithError("Child failed to open file");
        }

        fscanf(fileReader, "%d", &elementCount);

        int readIndex = 0;

        while (readIndex < elementCount)
        {
            fscanf(fileReader, "%d", &numberArray[readIndex]);
            readIndex++;
        }

        fclose(fileReader);

        sortArrayAscending(numberArray, elementCount);

        FILE *fileUpdater = fopen(DATA_FILE, "w");

        if (fileUpdater == NULL)
        {
            terminateWithError("Child failed to update file");
        }

        fprintf(fileUpdater, "%d\n", elementCount);

        int updateIndex = 0;

        while (updateIndex < elementCount)
        {
            fprintf(fileUpdater, "%d ", numberArray[updateIndex]);
            updateIndex++;
        }

        fclose(fileUpdater);
        exit(EXIT_SUCCESS);
    }
    else
    {
        wait(NULL);

        FILE *fileReader = fopen(DATA_FILE, "r");

        if (fileReader == NULL)
        {
            terminateWithError("Parent failed to read sorted data");
        }

        fscanf(fileReader, "%d", &elementCount);

        int readIndex = 0;

        while (readIndex < elementCount)
        {
            fscanf(fileReader, "%d", &numberArray[readIndex]);
            readIndex++;
        }

        fclose(fileReader);

        printf("Array after sorting:\n");

        int finalIndex = 0;
        
        while (finalIndex < elementCount)
        {
            printf("%d ", numberArray[finalIndex]);
            finalIndex++;
        }
        printf("\n");
    }

    return 0;
}
