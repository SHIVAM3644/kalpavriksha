#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_ELEMENTS 50
#define MESSAGE_TYPE_INPUT 1
#define MESSAGE_TYPE_OUTPUT 2

typedef struct MessageBuffer
{
    long messageType;
    int elementCount;
    int numberArray[MAX_ELEMENTS];
} MessageBuffer;

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
    key_t messageQueueKey;
    int messageQueueId;
    pid_t childProcessId;
    MessageBuffer messageData;

    messageQueueKey = ftok(".", 65);

    if (messageQueueKey == -1)
    {
        terminateWithError("Failed to generate key");
    }

    messageQueueId = msgget(messageQueueKey, IPC_CREAT | 0666);

    if (messageQueueId == -1)
    {
        terminateWithError("Failed to create message queue");
    }

    printf("Enter number of elements: ");

    if (scanf("%d", &messageData.elementCount) != 1 ||
        messageData.elementCount <= 0 ||
        messageData.elementCount > MAX_ELEMENTS)
    {
        terminateWithError("Invalid element count");
    }

    printf("Enter %d integers:\n", messageData.elementCount);

    int inputIndex = 0;

    while (inputIndex < messageData.elementCount)
    {
        if (scanf("%d", &messageData.numberArray[inputIndex]) != 1)
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

    while (displayIndex < messageData.elementCount)
    {
        printf("%d ", messageData.numberArray[displayIndex]);
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
        MessageBuffer receivedData;

        msgrcv(messageQueueId,
               &receivedData,
               sizeof(MessageBuffer) - sizeof(long),
               MESSAGE_TYPE_INPUT,
               0);

        sortArrayAscending(receivedData.numberArray, receivedData.elementCount);

        receivedData.messageType = MESSAGE_TYPE_OUTPUT;

        msgsnd(messageQueueId,
               &receivedData,
               sizeof(MessageBuffer) - sizeof(long),
               0);

        exit(EXIT_SUCCESS);
    }
    else
    {
        messageData.messageType = MESSAGE_TYPE_INPUT;

        msgsnd(messageQueueId,
               &messageData,
               sizeof(MessageBuffer) - sizeof(long),
               0);

        wait(NULL);

        msgrcv(messageQueueId,
               &messageData,
               sizeof(MessageBuffer) - sizeof(long),
               MESSAGE_TYPE_OUTPUT,
               0);

        printf("Array after sorting:\n");

        int finalIndex = 0;
        while (finalIndex < messageData.elementCount)
        {
            printf("%d ", messageData.numberArray[finalIndex]);
            finalIndex++;
        }
        printf("\n");

        msgctl(messageQueueId, IPC_RMID, NULL);
    }

    return 0;
}
