#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_ELEMENTS 50

void terminateWithError(const char *errorMessage)
{
    fprintf(stderr, "ERROR: %s\n", errorMessage);
    exit(EXIT_FAILURE);
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
    int pipeParentToChild[2];
    int pipeChildToParent[2];
    pid_t childProcessId;

    printf("Enter number of elements: ");
    
    if (scanf("%d", &elementCount) != 1 || elementCount <= 0 || elementCount > MAX_ELEMENTS)
    {
        terminateWithError("Invalid element count");
    }

    printf("Enter %d integers:\n", elementCount);

    int inputIndex = 0;

    while (inputIndex < elementCount)
    {
        if (scanf("%d", &numberArray[inputIndex]) != 1)
        {
            terminateWithError("Invalid integer input");
        }

        inputIndex++;
    }

    int extraCharacter = getchar();

    if (extraCharacter != '\n' && extraCharacter != EOF)
    {
        terminateWithError("Extra input detected");
    }

    if (pipe(pipeParentToChild) == -1 || pipe(pipeChildToParent) == -1)
    {
        terminateWithError("Pipe creation failed");
    }

    childProcessId = fork();

    if (childProcessId < 0)
    {
        terminateWithError("Fork failed");
    }

    if (childProcessId == 0)
    {
        close(pipeParentToChild[1]);
        close(pipeChildToParent[0]);

        read(pipeParentToChild[0], &elementCount, sizeof(int));
        read(pipeParentToChild[0], numberArray, sizeof(int) * elementCount);

        sortArrayAscending(numberArray, elementCount);

        write(pipeChildToParent[1], numberArray, sizeof(int) * elementCount);

        close(pipeParentToChild[0]);
        close(pipeChildToParent[1]);
        exit(EXIT_SUCCESS);
    }
    else
    {
        close(pipeParentToChild[0]);
        close(pipeChildToParent[1]);

        write(pipeParentToChild[1], &elementCount, sizeof(int));
        write(pipeParentToChild[1], numberArray, sizeof(int) * elementCount);

        read(pipeChildToParent[0], numberArray, sizeof(int) * elementCount);

        close(pipeParentToChild[1]);
        close(pipeChildToParent[0]);

        wait(NULL);

        printf("Sorted array:\n");
        int displayIndex = 0;

        while (displayIndex < elementCount)
        {
            printf("%d ", numberArray[displayIndex]);
            displayIndex++;
        }
        printf("\n");
    }

    return 0;
}
