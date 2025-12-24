#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#define SERVER_PORT 9090
#define BUFFER_SIZE 256
#define ACCOUNT_FILE "../resource/accountDB.txt"

pthread_mutex_t accountMutex;

void terminateWithError(const char *errorMessage)
{
    perror(errorMessage);
    exit(EXIT_FAILURE);
}

int readAccountBalance(void)
{
    FILE *filePointer = fopen(ACCOUNT_FILE, "r");

    if (filePointer == NULL)
    {
        return -1;
    }

    int balance;

    if (fscanf(filePointer, "%d", &balance) != 1)
    {
        fclose(filePointer);
        return -1;
    }

    fclose(filePointer);
    return balance;
}

int writeAccountBalance(int balance)
{
    FILE *filePointer = fopen(ACCOUNT_FILE, "w");

    if (filePointer == NULL)
    {
        return -1;
    }

    fprintf(filePointer, "%d\n", balance);
    fclose(filePointer);

    return 0;
}

void *handleClient(void *clientSocketArgument)
{
    int clientSocketDescriptor = *(int *)clientSocketArgument;
    free(clientSocketArgument);

    char requestBuffer[BUFFER_SIZE];
    char responseBuffer[BUFFER_SIZE];

    memset(requestBuffer, 0, sizeof(requestBuffer));
    recv(clientSocketDescriptor, requestBuffer, sizeof(requestBuffer), 0);

    int operationCode;
    int transactionAmount = 0;

    if (sscanf(requestBuffer, "%d %d", &operationCode, &transactionAmount) < 1)
    {
        send(clientSocketDescriptor, "ERROR: Invalid request\n", 23, 0);
        close(clientSocketDescriptor);
        return NULL;
    }

    pthread_mutex_lock(&accountMutex);

    int currentBalance = readAccountBalance();

    if (currentBalance < 0)
    {
        pthread_mutex_unlock(&accountMutex);
        send(clientSocketDescriptor, "ERROR: Account read failed\n", 27, 0);
        close(clientSocketDescriptor);
        return NULL;
    }

    if (operationCode == 1)
    {
        if (transactionAmount <= 0)
        {
            snprintf(responseBuffer, BUFFER_SIZE, "ERROR: Invalid withdraw amount\n");
        }
        else if (transactionAmount > currentBalance)
        {
            snprintf(responseBuffer, BUFFER_SIZE, "FAILED: Insufficient balance\n");
        }
        else
        {
            currentBalance -= transactionAmount;
            writeAccountBalance(currentBalance);
            snprintf(responseBuffer, BUFFER_SIZE,
                     "SUCCESS: Withdrawn %d | Balance %d\n",
                     transactionAmount, currentBalance);
        }
    }
    else if (operationCode == 2)
    {
        if (transactionAmount <= 0)
        {
            snprintf(responseBuffer, BUFFER_SIZE, "ERROR: Invalid deposit amount\n");
        }
        else
        {
            currentBalance += transactionAmount;
            writeAccountBalance(currentBalance);
            snprintf(responseBuffer, BUFFER_SIZE,
                     "SUCCESS: Deposited %d | Balance %d\n",
                     transactionAmount, currentBalance);
        }
    }
    else if (operationCode == 3)
    {
        snprintf(responseBuffer, BUFFER_SIZE,
                 "CURRENT BALANCE: %d\n", currentBalance);
    }
    else
    {
        snprintf(responseBuffer, BUFFER_SIZE, "ERROR: Invalid operation\n");
    }

    pthread_mutex_unlock(&accountMutex);

    send(clientSocketDescriptor, responseBuffer, strlen(responseBuffer), 0);
    close(clientSocketDescriptor);
    return NULL;
}

int main(void)
{
    int serverSocketDescriptor;
    struct sockaddr_in serverAddress;

    pthread_mutex_init(&accountMutex, NULL);

    serverSocketDescriptor = socket(AF_INET, SOCK_STREAM, 0);
    
    if (serverSocketDescriptor < 0)
    {
        terminateWithError("Socket creation failed");
    }

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(SERVER_PORT);

    if (bind(serverSocketDescriptor,
             (struct sockaddr *)&serverAddress,
             sizeof(serverAddress)) < 0)
    {
        terminateWithError("Bind failed");
    }

    if (listen(serverSocketDescriptor, 5) < 0)
    {
        terminateWithError("Listen failed");
    }

    printf("ATM Server running on port %d...\n", SERVER_PORT);

    while (1)
    {
        int *clientSocketDescriptor = malloc(sizeof(int));
        *clientSocketDescriptor = accept(serverSocketDescriptor, NULL, NULL);

        pthread_t clientThread;
        pthread_create(&clientThread, NULL,
                       handleClient,
                       clientSocketDescriptor);
        pthread_detach(clientThread);
    }

    close(serverSocketDescriptor);
    pthread_mutex_destroy(&accountMutex);
    return 0;
}
