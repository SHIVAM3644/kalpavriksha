#include <stdio.h>
#include <string.h>


struct UserRecord
{
    int userId;
    char userName[50];
    int userAge;
};


void addUserRecord();
void deleteUserRecord();
void updateUserRecord();
void displayAllRecords();

int main()
{
    int menuChoice;

    do
    {
        printf("\n--- User Record Management System ---\n");
        printf("1. Add New User\n");
        printf("2. Delete Existing User\n");
        printf("3. Update User Details\n");
        printf("4. Display All Users\n");
        printf("5. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &menuChoice);
        getchar(); // consume newline

        switch (menuChoice)
        {
            case 1:
            {
                addUserRecord();
                break;
            }
            case 2:
            {
                deleteUserRecord();
                break;
            }
            case 3:
            {
                updateUserRecord();
                break;
            }
            case 4:
            {
                displayAllRecords();
                break;
            }
            case 5:
            {
                printf("Exiting program...\n");
                break;
            }
            default:
            {
                printf("Invalid choice! Please select a valid option.\n");
                break;
            }
        }

    } while (menuChoice != 5);

    return 0;
}


void addUserRecord()
{
    struct UserRecord newUser;
    FILE *filePointer;
    int existingUserId, isDuplicate = 0;
    char fileLine[200];

    printf("Enter User ID: ");
    scanf("%d", &newUser.userId);
    getchar();

    printf("Enter User Name: ");
    fgets(newUser.userName, sizeof(newUser.userName), stdin);

    printf("Enter User Age: ");
    scanf("%d", &newUser.userAge);
    getchar();

    
    filePointer = fopen("users.txt", "r");

    if (filePointer != NULL)
    {
        while (fgets(fileLine, sizeof(fileLine), filePointer))
        {
            if (sscanf(fileLine, "ID: %d", &existingUserId) == 1)
            {
                if (existingUserId == newUser.userId)
                {
                    isDuplicate = 1;
                    break;
                }
            }
        }
        fclose(filePointer);
    }

    if (isDuplicate)
    {
        printf("Record with ID %d already exists.\n", newUser.userId);
        return;
    }

    filePointer = fopen("users.txt", "a");

    if (filePointer == NULL)
    {
        printf("Error opening file for writing!\n");
        return;
    }

    fprintf(filePointer, "ID: %d\nName: %sAge: %d\n\n", newUser.userId, newUser.userName, newUser.userAge);
    fclose(filePointer);

    printf("User record added successfully.\n");
}


void deleteUserRecord()
{
    int targetUserId, currentUserId;
    char fileLine[200];
    int linesToSkip = 0;
    FILE *filePointer, *tempFile;

    printf("Enter User ID to delete: ");
    scanf("%d", &targetUserId);

    filePointer = fopen("users.txt", "r");

    if (!filePointer)
    {
        printf("File not found!\n");
        return;
    }

    tempFile = fopen("temp.txt", "w");

    if (!tempFile)
    {
        printf("Error creating temporary file!\n");
        fclose(filePointer);
        return;
    }

    while (fgets(fileLine, sizeof(fileLine), filePointer))
    {
        if (sscanf(fileLine, "ID: %d", &currentUserId) == 1)
        {
            if (currentUserId == targetUserId)
            {
                linesToSkip = 3;
                continue;
            }
        }

        if (linesToSkip > 0)
        {
            linesToSkip--;
            continue;
        }

        fputs(fileLine, tempFile);
    }

    fclose(filePointer);
    fclose(tempFile);

    remove("users.txt");
    rename("temp.txt", "users.txt");

    printf("Record with ID %d deleted successfully.\n", targetUserId);
}


void updateUserRecord()
{
    int targetUserId, currentUserId, linesToSkip = 0;
    char fileLine[200], updatedName[50];
    int updatedAge;
    FILE *filePointer, *tempFile;
    int recordFound = 0;

    printf("Enter User ID to update: ");
    scanf("%d", &targetUserId);
    getchar();

    filePointer = fopen("users.txt", "r");

    if (!filePointer)
    {
        printf("File not found!\n");
        return;
    }

    tempFile = fopen("temp.txt", "w");

    if (!tempFile)
    {
        printf("Error creating temporary file!\n");
        fclose(filePointer);
        return;
    }

    while (fgets(fileLine, sizeof(fileLine), filePointer))
    {
        if (sscanf(fileLine, "ID: %d", &currentUserId) == 1)
        {
            if (currentUserId == targetUserId)
            {
                recordFound = 1;

                printf("Enter new User Name: ");
                fgets(updatedName, sizeof(updatedName), stdin);

                printf("Enter new User Age: ");
                scanf("%d", &updatedAge);
                getchar();

                fprintf(tempFile, "ID: %d\nName: %sAge: %d\n\n", currentUserId, updatedName, updatedAge);

                linesToSkip = 2;
                continue;
            }
        }

        if (linesToSkip > 0)
        {
            linesToSkip--;
            continue;
        }

        fputs(fileLine, tempFile);
    }

    fclose(filePointer);
    fclose(tempFile);

    if (!recordFound)
    {
        printf("No record found with ID %d.\n", targetUserId);
        remove("temp.txt");
        return;
    }

    remove("users.txt");
    rename("temp.txt", "users.txt");

    printf("Record with ID %d updated successfully.\n", targetUserId);
}


void displayAllRecords()
{
    FILE *filePointer;
    char character;

    filePointer = fopen("users.txt", "r");

    if (filePointer == NULL)
    {
        printf("File not found!\n");
        return;
    }

    printf("\n--- All User Records ---\n");

    while ((character = fgetc(filePointer)) != EOF)
    {
        printf("%c", character);
    }

    fclose(filePointer);
}
