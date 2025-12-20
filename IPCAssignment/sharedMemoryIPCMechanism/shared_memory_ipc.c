#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_ELEMENTS 50

typedef struct SharedData
{
    int elementCount;
    int numberArray[MAX_ELEMENTS];
} SharedData;

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
    key_t sharedMemoryKey;
    int sharedMemoryId;
    SharedData *sharedMemoryData;
    pid_t childProcessId;

    sharedMemoryKey = ftok(".", 75);

    if (sharedMemoryKey == -1)
    {
        terminateWithError("Failed to generate key");
    }

    sharedMemoryId = shmget(sharedMemoryKey,
                            sizeof(SharedData),
                            IPC_CREAT | 0666);

    if (sharedMemoryId == -1)
    {
        terminateWithError("Failed to create shared memory");
    }

    sharedMemoryData = (SharedData *)shmat(sharedMemoryId, NULL, 0);

    if (sharedMemoryData == (void *)-1)
    {
        terminateWithError("Failed to attach shared memory");
    }

    printf("Enter number of elements: ");

    if (scanf("%d", &sharedMemoryData->elementCount) != 1 ||
        sharedMemoryData->elementCount <= 0 ||
        sharedMemoryData->elementCount > MAX_ELEMENTS)
    {
        terminateWithError("Invalid element count");
    }

    printf("Enter %d integers:\n", sharedMemoryData->elementCount);

    int inputIndex = 0;

    while (inputIndex < sharedMemoryData->elementCount)
    {
        if (scanf("%d", &sharedMemoryData->numberArray[inputIndex]) != 1)
        {
            terminateWithError("Invalid integer input");
        }
        inputIndex++;
    }

    int extraCharacter;
    extraCharacter = getchar();

    if (extraCharacter != '\n' && extraCharacter != EOF)
    {
        terminateWithError("Extra input detected");
    }


    printf("Array before sorting:\n");

    int displayIndex = 0;

    while (displayIndex < sharedMemoryData->elementCount)
    {
        printf("%d ", sharedMemoryData->numberArray[displayIndex]);
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
        sortArrayAscending(sharedMemoryData->numberArray,
                           sharedMemoryData->elementCount);
        exit(EXIT_SUCCESS);
    }
    else
    {
        wait(NULL);

        printf("Array after sorting:\n");

        int finalIndex = 0;

        while (finalIndex < sharedMemoryData->elementCount)
        {
            printf("%d ", sharedMemoryData->numberArray[finalIndex]);
            finalIndex++;
        }
        
        printf("\n");

        shmdt(sharedMemoryData);
        shmctl(sharedMemoryId, IPC_RMID, NULL);
    }

    return 0;
}
