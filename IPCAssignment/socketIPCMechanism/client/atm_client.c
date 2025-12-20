#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_PORT 9090
#define BUFFER_SIZE 256

void terminateWithError(const char *errorMessage)
{
    fprintf(stderr, "ERROR: %s\n", errorMessage);
    exit(EXIT_FAILURE);
}

int main(void)
{
    int clientSocketDescriptor;
    struct sockaddr_in serverAddress;
    char requestBuffer[BUFFER_SIZE];
    char responseBuffer[BUFFER_SIZE];
    char inputBuffer[BUFFER_SIZE];

    while (1)
    {
        clientSocketDescriptor = socket(AF_INET, SOCK_STREAM, 0);
        
        if (clientSocketDescriptor < 0)
        {
            terminateWithError("Socket creation failed");
        }

        serverAddress.sin_family = AF_INET;
        serverAddress.sin_port = htons(SERVER_PORT);

        if (inet_pton(AF_INET, "127.0.0.1", &serverAddress.sin_addr) <= 0)
        {
            terminateWithError("Invalid server address");
        }

        if (connect(clientSocketDescriptor,
                    (struct sockaddr *)&serverAddress,
                    sizeof(serverAddress)) < 0)
        {
            terminateWithError("Connection failed");
        }

        printf("\n1. Withdraw\n2. Deposit\n3. Display Balance\n4. Exit\n");
        printf("Enter choice: ");

        int userChoice = 0;
        int amount = 0;

        if (fgets(inputBuffer, sizeof(inputBuffer), stdin) == NULL ||
            sscanf(inputBuffer, "%d", &userChoice) != 1)
        {
            printf("Invalid choice input\n");
            close(clientSocketDescriptor);
            continue;
        }

        if (userChoice == 4)
        {
            close(clientSocketDescriptor);
            printf("Exiting ATM client...\n");
            break;
        }

        if (userChoice == 1 || userChoice == 2)
        {
            printf("Enter amount: ");

            if (fgets(inputBuffer, sizeof(inputBuffer), stdin) == NULL ||
                sscanf(inputBuffer, "%d", &amount) != 1 ||
                amount <= 0)
            {
                printf("Invalid amount\n");
                close(clientSocketDescriptor);
                continue;
            }
        }
        else if (userChoice != 3)
        {
            printf("Invalid menu option\n");
            close(clientSocketDescriptor);
            continue;
        }

        snprintf(requestBuffer, BUFFER_SIZE, "%d %d", userChoice, amount);
        send(clientSocketDescriptor, requestBuffer, strlen(requestBuffer), 0);

        memset(responseBuffer, 0, sizeof(responseBuffer));
        recv(clientSocketDescriptor, responseBuffer, sizeof(responseBuffer), 0);

        printf("Server Response: %s\n", responseBuffer);

        close(clientSocketDescriptor);
    }

    return 0;
}
