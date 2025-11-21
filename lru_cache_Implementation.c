#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_KEY 10000
#define MAX_STRING_SIZE 100

typedef struct Node 
{
    int key;
    char value[MAX_STRING_SIZE];
    struct Node* prev;
    struct Node* next;
} Node;

typedef struct 
{
    int capacity;
    int size;
    Node* head;
    Node* tail;
    Node* map[MAX_KEY];
} LRUCache;

Node* createNode(int key, const char* value) 
{
    Node* node = (Node*)malloc(sizeof(Node));

    if (node == NULL)
    {
        printf("Memory allocation failed\n");
        exit(1);
    }

    node->key = key;
    strcpy(node->value, value);
    node->prev = node->next = NULL;

    return node;
}

LRUCache* createCache(int capacity) 
{
    LRUCache* cache = (LRUCache*)malloc(sizeof(LRUCache));

    if (cache == NULL)
    {
        printf("Memory allocation failed\n");
        exit(1);
    }

    cache->capacity = capacity;
    cache->size = 0;
    cache->head = cache->tail = NULL;

    for (int mapIndex = 0; mapIndex < MAX_KEY; mapIndex++)
    {
        cache->map[mapIndex] = NULL;
    }

    return cache;
}

void moveToFront(LRUCache* cache, Node* node) 
{
    if (cache->head == node)
    {
        return;
    }

    if (node->prev)
    {
        node->prev->next = node->next;
    }

    if (node->next)
    {
        node->next->prev = node->prev;
    }

    if (cache->tail == node)
    {
        cache->tail = node->prev;
    }

    node->prev = NULL;
    node->next = cache->head;

    if (cache->head)
    {
        cache->head->prev = node;
    }

    cache->head = node;

    if (cache->tail == NULL)
    {
        cache->tail = node;
    }
}

void removeLRU(LRUCache* cache) 
{
    Node* lru = cache->tail;

    if (!lru)
    {
        return;
    }

    if (lru->prev)
    {
        lru->prev->next = NULL;
    }

    cache->tail = lru->prev;

    if (cache->tail == NULL)
    {
        cache->head = NULL;
    }

    cache->map[lru->key] = NULL;
    free(lru);
    cache->size--;
}

char* get(LRUCache* cache, int key) 
{
    if (key >= MAX_KEY || cache->map[key] == NULL)
    {
        return NULL;
    }

    Node* node = cache->map[key];
    moveToFront(cache, node);

    return node->value;
}

void put(LRUCache* cache, int key, const char* value) 
{
    if (cache->map[key] != NULL)
    {
        Node* node = cache->map[key];
        strcpy(node->value, value);
        moveToFront(cache, node);

        return;
    }

    Node* node = createNode(key, value);
    node->next = cache->head;

    if (cache->head)
    {
        cache->head->prev = node;
    }

    cache->head = node;

    if (cache->tail == NULL)
    {
        cache->tail = node;
    }

    cache->map[key] = node;
    cache->size++;

    if (cache->size > cache->capacity)
    {
        removeLRU(cache);
    }
}

int isInteger(const char* string)
{
    if (*string == '\0')
    {
        return 0;
    }

    for (int position = 0; string[position] != '\0'; position++)
    {
        if (string[position] < '0' || string[position] > '9')
        {
            return 0;
        }
    }

    return 1;
}

int main() 
{
    char inputBuffer[200];
    LRUCache* cacheInstance = NULL;

    while (1)
    {
        if (!fgets(inputBuffer, sizeof(inputBuffer), stdin))
        {
            break;
        }

        char commandWord[MAX_STRING_SIZE], firstArgument[MAX_STRING_SIZE], secondArgument[MAX_STRING_SIZE], extraArgument[MAX_STRING_SIZE];
        commandWord[0] = firstArgument[0] = secondArgument[0] = extraArgument[0] = '\0';

        int tokenCount = sscanf(inputBuffer, "%s %s %s %s",
                                commandWord, firstArgument, secondArgument, extraArgument);

        if (tokenCount <= 0)
        {
            continue;
        }

        if (strcmp(commandWord, "createCache") == 0)
        {
            if (tokenCount != 2)
            {
                printf("Invalid input: createCache requires exactly 1 argument\n");
                continue;
            }

            if (!isInteger(firstArgument))
            {
                printf("Invalid input: capacity must be an integer\n");
                continue;
            }

            int capacityValue = atoi(firstArgument);
            cacheInstance = createCache(capacityValue);
        }
        else if (strcmp(commandWord, "put") == 0)
        {
            if (tokenCount != 3)
            {
                printf("Invalid input: put requires <key> <value>\n");
                continue;
            }

            if (cacheInstance == NULL)
            {
                printf("Cache not initialized\n");
                continue;
            }

            if (!isInteger(firstArgument))
            {
                printf("Invalid input: key must be an integer\n");
                continue;
            }

            int keyValue = atoi(firstArgument);

            put(cacheInstance, keyValue, secondArgument);
        }
        else if (strcmp(commandWord, "get") == 0)
        {
            if (tokenCount != 2)
            {
                printf("Invalid input: get requires <key>\n");
                continue;
            }

            if (cacheInstance == NULL)
            {
                printf("Cache not initialized\n");
                continue;
            }

            if (!isInteger(firstArgument))
            {
                printf("Invalid input: key must be an integer\n");
                continue;
            }

            int keyValue = atoi(firstArgument);
            char* retrievedValue = get(cacheInstance, keyValue);

            if (retrievedValue)
            {
                printf("%s\n", retrievedValue);
            }
            else
            {
                printf("NULL\n");
            }
        }
        else if (strcmp(commandWord, "exit") == 0)
        {
            if (tokenCount != 1)
            {
                printf("Invalid input: exit takes no arguments\n");
                continue;
            }
            break;
        }
        else
        {
            printf("Invalid command\n");
        }
    }

    return 0;
}
