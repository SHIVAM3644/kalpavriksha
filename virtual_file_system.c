#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define BLOCK_SIZE 512
#define MAX_NAME_LENGTH 50
#define TOTAL_BLOCKS 1024

typedef struct FreeBlock
{
    int blockIndex;
    struct FreeBlock *next;
    struct FreeBlock *prev;
} FreeBlock;

typedef struct FileNode
{
    char name[MAX_NAME_LENGTH + 1];
    int isDirectory;
    struct FileNode *parentNode;
    struct FileNode *childHeadNode;
    struct FileNode *siblingNextNode;
    struct FileNode *siblingPrevNode;
    int *blockPointers;
    int blockCount;
    int sizeBytes;
} FileNode;

static char *virtualDisk;
static int freeBlockCount;
static FreeBlock *freeBlockListHead = NULL;
static FreeBlock *freeBlockListTail = NULL;
static FileNode *rootDirectoryNode = NULL;
static FileNode *currentDirectoryNode = NULL;

static char *copyString(const char *sourceString)
{
    char *copyString;
    copyString = malloc(strlen(sourceString) + 1);
    strcpy(copyString, sourceString);

    return copyString;
}

static char *trimWhitespace(char *inputString)
{
    char *endPointer;

    while (isspace((unsigned char)*inputString))
    {
        inputString++;
    }

    if (*inputString == '\0')
    {
        return inputString;
    }

    endPointer = inputString + strlen(inputString) - 1;

    while (endPointer > inputString && isspace((unsigned char)*endPointer))
    {
        endPointer--;
    }

    *(endPointer + 1) = '\0';

    return inputString;
}

static void initialize_free_blocks()
{
    int blockCounterIndex;
    freeBlockCount = 0;

    for (blockCounterIndex = 0; blockCounterIndex < TOTAL_BLOCKS; blockCounterIndex++)
    {
        FreeBlock *newFreeBlockNode;
        newFreeBlockNode = malloc(sizeof(FreeBlock));
        newFreeBlockNode->blockIndex = blockCounterIndex;
        newFreeBlockNode->next = NULL;
        newFreeBlockNode->prev = freeBlockListTail;

        if (freeBlockListTail)
        {
            freeBlockListTail->next = newFreeBlockNode;
        }
        else
        {
            freeBlockListHead = newFreeBlockNode;
        }

        freeBlockListTail = newFreeBlockNode;
        freeBlockCount++;
    }
}

static int allocate_block()
{
    FreeBlock *allocatedBlockNode;
    int allocatedBlockIndex;

    if (!freeBlockListHead)
    {
        return -1;
    }

    allocatedBlockNode = freeBlockListHead;
    allocatedBlockIndex = allocatedBlockNode->blockIndex;
    freeBlockListHead = allocatedBlockNode->next;

    if (freeBlockListHead)
    {
        freeBlockListHead->prev = NULL;
    }
    else
    {
        freeBlockListTail = NULL;
    }

    free(allocatedBlockNode);
    freeBlockCount--;

    return allocatedBlockIndex;
}

static void free_block(int blockIndexToFree)
{
    FreeBlock *freedBlockNode;
    freedBlockNode = malloc(sizeof(FreeBlock));

    freedBlockNode->blockIndex = blockIndexToFree;
    freedBlockNode->next = NULL;
    freedBlockNode->prev = freeBlockListTail;

    if (freeBlockListTail)
    {
        freeBlockListTail->next = freedBlockNode;
    }
    else
    {
        freeBlockListHead = freedBlockNode;
    }

    freeBlockListTail = freedBlockNode;
    freeBlockCount++;
}

static FileNode *create_node(const char *nodeName, int isDirectoryFlag)
{
    FileNode *newNode;
    newNode = malloc(sizeof(FileNode));

    strncpy(newNode->name, nodeName, MAX_NAME_LENGTH);
    newNode->name[MAX_NAME_LENGTH] = '\0';
    newNode->isDirectory = isDirectoryFlag;
    newNode->parentNode = NULL;
    newNode->childHeadNode = NULL;
    newNode->siblingNextNode = NULL;
    newNode->siblingPrevNode = NULL;
    newNode->blockPointers = NULL;
    newNode->blockCount = 0;
    newNode->sizeBytes = 0;

    return newNode;
}

static void add_child(FileNode *parentDirectoryNode, FileNode *childNode)
{
    FileNode *headNode;
    FileNode *tailNode;

    childNode->parentNode = parentDirectoryNode;

    if (parentDirectoryNode->childHeadNode == NULL)
    {
        parentDirectoryNode->childHeadNode = childNode;
        childNode->siblingNextNode = childNode;
        childNode->siblingPrevNode = childNode;

        return;
    }

    headNode = parentDirectoryNode->childHeadNode;
    tailNode = headNode->siblingPrevNode;

    tailNode->siblingNextNode = childNode;
    childNode->siblingPrevNode = tailNode;
    childNode->siblingNextNode = headNode;
    headNode->siblingPrevNode = childNode;
}

static FileNode *find_child(FileNode *directoryNode, const char *childName)
{
    FileNode *scanNode;

    if (!directoryNode->childHeadNode)
    {
        return NULL;
    }

    scanNode = directoryNode->childHeadNode;

    do
    {
        if (strcmp(scanNode->name, childName) == 0)
        {
            return scanNode;
        }

        scanNode = scanNode->siblingNextNode;
    }
    while (scanNode != directoryNode->childHeadNode);

    return NULL;
}

static void command_pwd()
{
    FileNode *stackNodes[100];
    int stackCount;
    FileNode *walkingNode;
    int indexCounter;

    if (currentDirectoryNode == rootDirectoryNode)
    {
        printf("/\n");
        return;
    }

    stackCount = 0;
    walkingNode = currentDirectoryNode;

    while (walkingNode != rootDirectoryNode)
    {
        stackNodes[stackCount++] = walkingNode;
        walkingNode = walkingNode->parentNode;
    }

    printf("/");

    for (indexCounter = stackCount - 1; indexCounter >= 0; indexCounter--)
    {
        printf("%s", stackNodes[indexCounter]->name);

        if (indexCounter > 0)
        {
            printf("/");
        }
    }

    printf("\n");
}

static void command_ls()
{
    FileNode *scanNode;

    if (!currentDirectoryNode->childHeadNode)
    {
        printf("(empty)\n");
        return;
    }

    scanNode = currentDirectoryNode->childHeadNode;

    do
    {
        printf("%s%s\n", scanNode->name, scanNode->isDirectory ? "/" : "");
        scanNode = scanNode->siblingNextNode;
    }
    while (scanNode != currentDirectoryNode->childHeadNode);
}

static void command_mkdir(const char *directoryName)
{
    if (!directoryName || *directoryName == '\0')
    {
        printf("Invalid command usage. Use: mkdir <dirname>\n");
        return;
    }

    if (find_child(currentDirectoryNode, directoryName))
    {
        printf("Name Already Exist In Current Directory.\n");
        return;
    }

    add_child(currentDirectoryNode, create_node(directoryName, 1));
    printf("Directory '%s' created successfully.\n", directoryName);
}

static void command_create(const char *fileName)
{
    if (!fileName || *fileName == '\0')
    {
        printf("Invalid command usage. Use: create <filename>\n");
        return;
    }

    if (find_child(currentDirectoryNode, fileName))
    {
        printf("Name already exists in current directory.\n");
        return;
    }

    add_child(currentDirectoryNode, create_node(fileName, 0));
    printf("File '%s' created successfully.\n", fileName);
}

static void command_cd(const char *targetName)
{
    FileNode *directoryPointer;

    if (!targetName)
    {
        printf("Invalid command.\n");
        return;
    }

    if (strcmp(targetName, "..") == 0)
    {
        if (currentDirectoryNode == rootDirectoryNode)
        {
            printf("Already in root directory.\n");
            return;
        }

        currentDirectoryNode = currentDirectoryNode->parentNode;

        if (currentDirectoryNode == rootDirectoryNode)
            printf("Moved to /\n");
        else
            printf("Moved to /%s\n", currentDirectoryNode->name);
        
        return;
    }

    directoryPointer = find_child(currentDirectoryNode, targetName);

    if (!directoryPointer || !directoryPointer->isDirectory)
    {
        printf("Directory not found.\n");
        return;
    }

    currentDirectoryNode = directoryPointer;

    if (currentDirectoryNode == rootDirectoryNode)
    {
        printf("Moved to /\n");
    }
    
    else
    {
        printf("Moved to /%s\n", currentDirectoryNode->name);

    }
}

static void command_write(const char *fileName, const char *content)
{
    int contentLength;
    int requiredBlocks;
    FileNode *fileNodePointer;
    int blockAllocationCounter;
    int offsetValue;
    int chunkSizeValue;

    if (!fileName || !content)
    {
        printf("Invalid usage. Use: write <filename> \"content\"\n");
        return;
    }

    contentLength = strlen(content);
    requiredBlocks = (contentLength + BLOCK_SIZE - 1) / BLOCK_SIZE;
    fileNodePointer = find_child(currentDirectoryNode, fileName);

    if (!fileNodePointer)
    {
        printf("File not present in this directory.\n");
        return;
    }

    if (fileNodePointer->isDirectory)
    {
        printf("Cannot write to a directory.\n");
        return;
    }

    for (blockAllocationCounter = 0; blockAllocationCounter < fileNodePointer->blockCount; blockAllocationCounter++)
    {
        free_block(fileNodePointer->blockPointers[blockAllocationCounter]);
    }

    free(fileNodePointer->blockPointers);
    fileNodePointer->blockPointers = NULL;
    fileNodePointer->blockCount = 0;

    if (contentLength == 0)
    {
        fileNodePointer->sizeBytes = 0;
        printf("Data written successfully (0 bytes).\n");
        return;
    }

    if (requiredBlocks > freeBlockCount)
    {
        printf("Error: Not enough disk space.\n");
        return;
    }

    fileNodePointer->blockPointers = malloc(requiredBlocks * sizeof(int));
    fileNodePointer->blockCount = requiredBlocks;
    fileNodePointer->sizeBytes = contentLength;

    for (blockAllocationCounter = 0; blockAllocationCounter < requiredBlocks; blockAllocationCounter++)
    {
        int newlyAllocatedIndex;

        newlyAllocatedIndex = allocate_block();
        offsetValue = blockAllocationCounter * BLOCK_SIZE;
        chunkSizeValue = (contentLength - offsetValue > BLOCK_SIZE) ? BLOCK_SIZE : (contentLength - offsetValue);
        fileNodePointer->blockPointers[blockAllocationCounter] = newlyAllocatedIndex;
        memcpy(virtualDisk + newlyAllocatedIndex * BLOCK_SIZE, content + offsetValue, chunkSizeValue);
    }

    printf("Data written successfully (%d bytes).\n", contentLength);
}

static void command_read(const char *fileName)
{
    FileNode *fileNodePointer;
    char *buffer;
    int positionIndex;
    int blockCounterIndex;
    int bytesToCopy;

    if (!fileName)
    {
        printf("Use: read <filename>\n");
        return;
    }

    fileNodePointer = find_child(currentDirectoryNode, fileName);

    if (!fileNodePointer || fileNodePointer->isDirectory)
    {
        printf("File not found.\n");
        return;
    }

    if (fileNodePointer->sizeBytes == 0)
    {
        printf("(empty)\n");
        return;
    }

    buffer = malloc(fileNodePointer->sizeBytes + 1);
    positionIndex = 0;

    for (blockCounterIndex = 0; blockCounterIndex < fileNodePointer->blockCount; blockCounterIndex++)
    {
        bytesToCopy = (fileNodePointer->sizeBytes - positionIndex > BLOCK_SIZE) ? BLOCK_SIZE : fileNodePointer->sizeBytes - positionIndex;
        memcpy(buffer + positionIndex, virtualDisk + fileNodePointer->blockPointers[blockCounterIndex] * BLOCK_SIZE, bytesToCopy);
        positionIndex += bytesToCopy;
    }

    buffer[fileNodePointer->sizeBytes] = '\0';

    printf("%s\n", buffer);
    free(buffer);
}

static void command_rmdir(const char *directoryName)
{
    FileNode *dirNode;

    if (!directoryName || *directoryName == '\0')
    {
        printf("Invalid usage. Use: rmdir <dirname>\n");
        return;
    }

    dirNode = find_child(currentDirectoryNode, directoryName);

    if (!dirNode || !dirNode->isDirectory)
    {
        printf("Directory not found.\n");
        return;
    }

    if (dirNode->childHeadNode != NULL)
    {
        printf("Directory not empty. Remove files first.\n");
        return;
    }

    if (dirNode->siblingNextNode == dirNode)
    {
        currentDirectoryNode->childHeadNode = NULL;
    }
    else
    {
        dirNode->siblingPrevNode->siblingNextNode = dirNode->siblingNextNode;
        dirNode->siblingNextNode->siblingPrevNode = dirNode->siblingPrevNode;

        if (currentDirectoryNode->childHeadNode == dirNode)
        {
            currentDirectoryNode->childHeadNode = dirNode->siblingNextNode;
        }
    }

    free(dirNode);
    printf("Directory removed successfully.\n");
}

static void command_delete(const char *fileName)
{
    FileNode *fileNodePointer;
    int blockCounterIndex;

    if (!fileName || *fileName == '\0')
    {
        printf("Use: delete <filename>\n");
        return;
    }

    fileNodePointer = find_child(currentDirectoryNode, fileName);

    if (!fileNodePointer || fileNodePointer->isDirectory)
    {
        printf("File not found.\n");
        return;
    }

    for (blockCounterIndex = 0; blockCounterIndex < fileNodePointer->blockCount; blockCounterIndex++)
    {
        free_block(fileNodePointer->blockPointers[blockCounterIndex]);
    }

    free(fileNodePointer->blockPointers);

    if (fileNodePointer->siblingNextNode == fileNodePointer)
    {
        currentDirectoryNode->childHeadNode = NULL;
    }
    else
    {
        fileNodePointer->siblingPrevNode->siblingNextNode = fileNodePointer->siblingNextNode;
        fileNodePointer->siblingNextNode->siblingPrevNode = fileNodePointer->siblingPrevNode;

        if (currentDirectoryNode->childHeadNode == fileNodePointer)
        {
            currentDirectoryNode->childHeadNode = fileNodePointer->siblingNextNode;
        }
    }

    free(fileNodePointer);
    printf("File deleted successfully.\n");
}

static void command_df()
{
    printf("Total Blocks: %d\n", TOTAL_BLOCKS);
    printf("Used Blocks: %d\n", TOTAL_BLOCKS - freeBlockCount);
    printf("Free Blocks: %d\n", freeBlockCount);
    printf("Disk Usage: %.2f%%\n", ((double)(TOTAL_BLOCKS - freeBlockCount) / TOTAL_BLOCKS) * 100.0);
}

static void parse_command(char *inputString, char **commandToken, char **argumentOne, char **argumentTwo)
{
    char *spacePointer;
    *commandToken = NULL;
    *argumentOne = NULL;
    *argumentTwo = NULL;
    inputString = trimWhitespace(inputString);

    if (*inputString == '\0')
    {
        return;
    }

    if (strncmp(inputString, "write ", 6) == 0)
    {
        inputString += 6;
        inputString = trimWhitespace(inputString);
        *argumentOne = inputString;

        while (*inputString && !isspace((unsigned char)*inputString))
        {
            inputString++;
        }

        if (*inputString == '\0')
        {
            return;
        }

        *inputString++ = '\0';
        inputString = trimWhitespace(inputString);

        if (*inputString != '"')
        {
            *commandToken = "INVALID";
            return;
        }

        inputString++;
        char *closingQuotePointer;
        closingQuotePointer = strrchr(inputString, '"');

        if (!closingQuotePointer)
        {
            *commandToken = "INVALID";
            return;
        }

        *closingQuotePointer = '\0';
        *commandToken = "write";
        *argumentTwo = copyString(inputString);
        return;
    }

    spacePointer = strchr(inputString, ' ');

    if (!spacePointer)
    {
        *commandToken = inputString;
        *argumentOne = NULL;
        return;
    }

    *spacePointer = '\0';
    *commandToken = inputString;
    {
        char *restPointer;
        restPointer = trimWhitespace(spacePointer + 1);

        if (*restPointer == '\0')
        {
            *argumentOne = NULL;
            return;
        }

        *argumentOne = restPointer;
    }
}

int main()
{
    char *inputLine;
    size_t inputBufferSize;
    char *commandToken;
    char *argumentOne;
    char *argumentTwo;

    virtualDisk = calloc(TOTAL_BLOCKS, BLOCK_SIZE);
    initialize_free_blocks();

    rootDirectoryNode = create_node("/", 1);
    currentDirectoryNode = rootDirectoryNode;

    printf("Compact VFS Ready. Type 'exit' to quit.\n");

    inputLine = (char*)malloc(inputBufferSize);
    inputBufferSize = 1024;

    while (1)
    {
        if (currentDirectoryNode == rootDirectoryNode)
        {
            printf("\n/ > ");
        }    
        else
        {

            printf("\n%s > ", currentDirectoryNode->name);
        }
    
        if (fgets(inputLine, inputBufferSize, stdin) == NULL)
        {
            break;
        }

        parse_command(inputLine, &commandToken, &argumentOne, &argumentTwo);

        if (!commandToken)
        {
            free(argumentTwo);
            continue;
        }

        if (strcmp(commandToken, "exit") == 0)
        {
            free(argumentTwo);
            break;
        }
        else if (strcmp(commandToken, "ls") == 0)
        {
            command_ls();
        }
        else if (strcmp(commandToken, "mkdir") == 0)
        {
            command_mkdir(argumentOne);
        }
        else if (strcmp(commandToken, "create") == 0)
        {
            command_create(argumentOne);
        }
        else if (strcmp(commandToken, "write") == 0)
        {
            command_write(argumentOne, argumentTwo);
        }
        else if (strcmp(commandToken, "read") == 0)
        {
            command_read(argumentOne);
        }
        else if (strcmp(commandToken, "delete") == 0)
        {
            command_delete(argumentOne);
        }
        else if (strcmp(commandToken, "rmdir") == 0)
        {
            command_rmdir(argumentOne);
        }
        else if (strcmp(commandToken, "cd") == 0)
        {
            command_cd(argumentOne);
        }
        else if (strcmp(commandToken, "pwd") == 0)
        {
            command_pwd();
        }
        else if (strcmp(commandToken, "df") == 0)
        {
            command_df();
        }
        else
        {
            printf("Invalid command.\n");
        }

        free(argumentTwo);
    }

    printf("Exiting... Memory Released.\n");
    free(virtualDisk);

    return 0;
}
