#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "Players_data.h"

#define MAX_CHARACTER_LENGTH 50
#define MAX_ARRAY_SIZE 50

typedef struct
{
    int playerId;
    char name[MAX_CHARACTER_LENGTH];
    char teamName[MAX_CHARACTER_LENGTH];
    char role[MAX_CHARACTER_LENGTH];
    int totalRuns;
    float battingAverage;
    float strikeRate;
    int wickets;
    float economyRate;
    float performanceIndex;
} PlayerModel;

typedef struct
{
    int teamId;
    char name[MAX_CHARACTER_LENGTH];
    int totalPlayers;
    float averageBattingStrikeRate;
    int playerIndexList[MAX_ARRAY_SIZE];
    int batsmanList[MAX_ARRAY_SIZE];
    int bowlerList[MAX_ARRAY_SIZE];
    int allrounderList[MAX_ARRAY_SIZE];
    int batsmanCount;
    int bowlerCount;
    int allrounderCount;
} TeamModel;

typedef struct
{
    int playerIndex;
    int teamIndex;
    int positionInList;
} HeapNode;

void loadPlayersData(PlayerModel allPlayers[], int *playerModelCount);
void loadTeamsData(TeamModel allTeams[], int *teamModelCount, PlayerModel allPlayers[], int playerModelCount);
int  searchTeamIndexById(TeamModel allTeams[], int teamId, int teamModelCount);

void sortPlayerIndexesByPerformance(PlayerModel allPlayers[], int indexArray[], int startIndex, int endIndex);
void mergeByPerformanceIndex(PlayerModel allPlayers[], int indexArray[], int startIndex, int midIndex, int endIndex);

void sortTeamsByStrikeRate(TeamModel allTeams[], int startIndex, int endIndex);
void mergeTeamSegmentsByStrikeRate(TeamModel allTeams[], int startIndex, int middleIndex, int endIndex);

void displayPlayersByTeamId(TeamModel allTeams[], PlayerModel allPlayers[], int teamId , int teamModelCount);
void displayTeamsByAverageStrikeRate(TeamModel allTeams[], int teamModelCount);

void displayTopKPlayersByRole(TeamModel allTeams[], PlayerModel allPlayers[], int teamModelCount);

void swapHeapNodes(HeapNode *node1, HeapNode *node2);
void heapifyDown(HeapNode heap[], int heapSize, int currentIndex, PlayerModel allPlayers[]);
void insertIntoHeap(HeapNode heap[], int *heapSize, HeapNode newNode, PlayerModel allPlayers[]);
HeapNode popMaxPerformancePlayer(HeapNode heap[], int *heapSize, PlayerModel allPlayers[]);

void displayPlayersOfAllTeamsByRole(TeamModel allTeams[], PlayerModel allPlayers[], int roleChoice, int teamModelCount);

int validateIntegerInput(char *inputString);
int validateFloatInput(char *inputString);
int getValidatedInt();
float getValidatedFloat();

int main()
{
    PlayerModel allPlayers[playerCount];
    TeamModel   allTeams[teamCount];
    int playerModelCount = 0;
    int teamModelCount = 0;
    int userChoice;

    loadPlayersData(allPlayers, &playerModelCount);
    loadTeamsData(allTeams, &teamModelCount, allPlayers, playerModelCount);

    do
    {
        printf("\n==============================================================================\n");
        printf("ICC ODI Player Performance Analyzer\n");
        printf("==============================================================================\n\n");

        printf("1. Display Players of a Specific Team\n");
        printf("2. Display Teams by Average Batting Strike Rate\n");
        printf("3. Display Top K Players of a Specific Team by Role\n");
        printf("4. Display all Players of a Specific Role Across All Teams\n");
        printf("5. Exit\n");
        printf("==============================================================================\n");
        printf("Enter your choice: ");

        userChoice = getValidatedInt();

        if(userChoice == 1) 
        {
            int inputTeamId;

            printf("\nEnter Team ID: ");
            inputTeamId = getValidatedInt();

            displayPlayersByTeamId(allTeams, allPlayers, inputTeamId, teamModelCount);
        }
        else if(userChoice == 2)
        {
            displayTeamsByAverageStrikeRate(allTeams, teamModelCount);
        }
        else if(userChoice == 3)
        {
            displayTopKPlayersByRole(allTeams, allPlayers, teamModelCount);
        }
        else if(userChoice == 4)
        {
            int roleChoice;

            printf("\nEnter Role (1-Batsman, 2-Bowler, 3-All-rounder): ");
            roleChoice = getValidatedInt();

            if(roleChoice < 1 || roleChoice > 3)
            {
                printf("Error: Invalid role choice.\n");
            }
            else
            {
                displayPlayersOfAllTeamsByRole(allTeams, allPlayers, roleChoice, teamModelCount);
            }
        }
        else if(userChoice == 5)
        {
            printf("\nExiting Program...\n");
        }
        else
        {
            printf("\nPlease Enter Valid Option\n");
        }

    } while(userChoice != 5);

    return 0;
}

void loadPlayersData(PlayerModel allPlayers[], int *playerModelCount)
{
    *playerModelCount = 0;

    for(int playerLoopIndex = 0; playerLoopIndex < playerCount; playerLoopIndex++)
    {
        allPlayers[playerLoopIndex].playerId       = players[playerLoopIndex].id;

        strcpy(allPlayers[playerLoopIndex].name, players[playerLoopIndex].name);
        strcpy(allPlayers[playerLoopIndex].teamName, players[playerLoopIndex].team);
        strcpy(allPlayers[playerLoopIndex].role, players[playerLoopIndex].role);

        allPlayers[playerLoopIndex].totalRuns      = players[playerLoopIndex].totalRuns;
        allPlayers[playerLoopIndex].battingAverage = players[playerLoopIndex].battingAverage;
        allPlayers[playerLoopIndex].strikeRate     = players[playerLoopIndex].strikeRate;
        allPlayers[playerLoopIndex].wickets        = players[playerLoopIndex].wickets;
        allPlayers[playerLoopIndex].economyRate    = players[playerLoopIndex].economyRate;

        if(strcmp(players[playerLoopIndex].role, "Batsman") == 0)
        {
            allPlayers[playerLoopIndex].performanceIndex =
                (players[playerLoopIndex].battingAverage * players[playerLoopIndex].strikeRate) / 100.0f;
        }
        else if(strcmp(players[playerLoopIndex].role, "Bowler") == 0)
        {
            allPlayers[playerLoopIndex].performanceIndex =
                (players[playerLoopIndex].wickets * 2) + (100 - players[playerLoopIndex].economyRate);
        }
        else
        {
            allPlayers[playerLoopIndex].performanceIndex =
                ((players[playerLoopIndex].battingAverage * players[playerLoopIndex].strikeRate) / 100.0f)
                + (players[playerLoopIndex].wickets * 2);
        }

        (*playerModelCount)++;
    }
}

void mergeByPerformanceIndex(PlayerModel allPlayers[], int indexArray[], int startIndex, int midIndex, int endIndex)
{
    int leftArraySize = midIndex - startIndex + 1;
    int rightArraySize = endIndex - midIndex;

    int leftArray[leftArraySize];
    int rightArray[rightArraySize];

    for(int leftArrayIndex = 0; leftArrayIndex < leftArraySize; leftArrayIndex++)
    {
        leftArray[leftArrayIndex] = indexArray[startIndex + leftArrayIndex];
    }

    for(int rightArrayIndex = 0; rightArrayIndex < rightArraySize; rightArrayIndex++)
    {
        rightArray[rightArrayIndex] = indexArray[midIndex + 1 + rightArrayIndex];
    }

    int leftArrayPosition = 0;
    int rightArrayPosition = 0;
    int writeArrayPos = startIndex;

    while(leftArrayPosition < leftArraySize && rightArrayPosition < rightArraySize)
    {
        if(allPlayers[leftArray[leftArrayPosition]].performanceIndex >= allPlayers[rightArray[rightArrayPosition]].performanceIndex)
        {
            indexArray[writeArrayPos++] = leftArray[leftArrayPosition++];        
        }
        else
        {
            indexArray[writeArrayPos++] = rightArray[rightArrayPosition++];            
        }

    }

    while(leftArrayPosition < leftArraySize)
    {
        indexArray[writeArrayPos++] = leftArray[leftArrayPosition++];
    }

    while(rightArrayPosition < rightArraySize)
    {
        indexArray[writeArrayPos++] = rightArray[rightArrayPosition++];
    }
}

void sortPlayerIndexesByPerformance(PlayerModel allPlayers[], int indexArray[], int startIndex, int endIndex)
{
    if(startIndex < endIndex)
    {
        int midIndex = startIndex + (endIndex - startIndex) / 2;

        sortPlayerIndexesByPerformance(allPlayers, indexArray, startIndex, midIndex);
        sortPlayerIndexesByPerformance(allPlayers, indexArray, midIndex + 1, endIndex);
        mergeByPerformanceIndex(allPlayers, indexArray, startIndex, midIndex, endIndex);
    }
}

void loadTeamsData(TeamModel allTeams[], int *teamModelCount, PlayerModel allPlayers[], int playerModelCount)
{
    *teamModelCount = 0;

    for(int teamLoopIndex = 0; teamLoopIndex < teamCount; teamLoopIndex++)
    {
        allTeams[teamLoopIndex].teamId  = teamLoopIndex + 1;
        strcpy(allTeams[teamLoopIndex].name, teams[teamLoopIndex]);
        allTeams[teamLoopIndex].totalPlayers = 0;
        allTeams[teamLoopIndex].averageBattingStrikeRate = 0.0f;

        allTeams[teamLoopIndex].batsmanCount = 0;
        allTeams[teamLoopIndex].bowlerCount = 0;
        allTeams[teamLoopIndex].allrounderCount = 0;

        *teamModelCount += 1;
    }

    for(int playerLoopIndex = 0; playerLoopIndex < playerModelCount; playerLoopIndex++)
    {
        for(int teamLoopIndex = 0; teamLoopIndex < *teamModelCount; teamLoopIndex++)
        {
            if(strcmp(allPlayers[playerLoopIndex].teamName, allTeams[teamLoopIndex].name) == 0)
            {
                int currentTeamPlayerCount = allTeams[teamLoopIndex].totalPlayers;

                allTeams[teamLoopIndex].playerIndexList[currentTeamPlayerCount] = playerLoopIndex;
                allTeams[teamLoopIndex].totalPlayers++;

                if(strcmp(allPlayers[playerLoopIndex].role, "Batsman") == 0)
                {
                    int batsmanInsertIndex = allTeams[teamLoopIndex].batsmanCount;

                    allTeams[teamLoopIndex].batsmanList[batsmanInsertIndex] = playerLoopIndex;
                    allTeams[teamLoopIndex].batsmanCount++;
                }
                else if(strcmp(allPlayers[playerLoopIndex].role, "Bowler") == 0)
                {
                    int bowlerInsertIndex = allTeams[teamLoopIndex].bowlerCount;

                    allTeams[teamLoopIndex].bowlerList[bowlerInsertIndex] = playerLoopIndex;
                    allTeams[teamLoopIndex].bowlerCount++;
                }
                else
                {
                    int allrounderInsertIndex = allTeams[teamLoopIndex].allrounderCount;

                    allTeams[teamLoopIndex].allrounderList[allrounderInsertIndex] = playerLoopIndex;
                    allTeams[teamLoopIndex].allrounderCount++;
                }

                break;
            }
        }
    }

    for(int teamLoopIndex = 0; teamLoopIndex < *teamModelCount; teamLoopIndex++)
    {
        float strikeRateSum = 0.0f;
        int eligiblePlayerCount = 0;

        for(int teamPlayerLoopIndex = 0; teamPlayerLoopIndex < allTeams[teamLoopIndex].totalPlayers; teamPlayerLoopIndex++)
        {
            int storedPlayerIndex = allTeams[teamLoopIndex].playerIndexList[teamPlayerLoopIndex];

            if(strcmp(allPlayers[storedPlayerIndex].role, "Batsman") == 0 ||
               strcmp(allPlayers[storedPlayerIndex].role, "All-rounder") == 0)
            {
                strikeRateSum += allPlayers[storedPlayerIndex].strikeRate;
                eligiblePlayerCount++;
            }
        }

        if(eligiblePlayerCount > 0)
        {
            allTeams[teamLoopIndex].averageBattingStrikeRate = strikeRateSum / eligiblePlayerCount;
        }
        else
        {
            allTeams[teamLoopIndex].averageBattingStrikeRate = 0.0f;            
        }

        if(allTeams[teamLoopIndex].batsmanCount > 0)
        {
            sortPlayerIndexesByPerformance(allPlayers, allTeams[teamLoopIndex].batsmanList, 0, allTeams[teamLoopIndex].batsmanCount - 1);           
        }
        if(allTeams[teamLoopIndex].bowlerCount > 0)
        {
            sortPlayerIndexesByPerformance(allPlayers, allTeams[teamLoopIndex].bowlerList, 0, allTeams[teamLoopIndex].bowlerCount - 1);

        }
        if(allTeams[teamLoopIndex].allrounderCount > 0)
        {
            sortPlayerIndexesByPerformance(allPlayers, allTeams[teamLoopIndex].allrounderList, 0, allTeams[teamLoopIndex].allrounderCount - 1);
        }
    }
}

int searchTeamIndexById(TeamModel allTeams[], int teamId, int teamModelCount)
{
    int searchLowIndex = 0;
    int searchHighIndex = teamModelCount - 1;

    while(searchLowIndex <= searchHighIndex)
    {
        int searchMidIndex = searchLowIndex + (searchHighIndex - searchLowIndex) / 2;

        if(allTeams[searchMidIndex].teamId == teamId)
        {
            return searchMidIndex;
        }

        if(allTeams[searchMidIndex].teamId < teamId)
        {
            searchLowIndex = searchMidIndex + 1;            
        }
        else
        {
            searchHighIndex = searchMidIndex - 1;            
        }
    }
    return -1;
}

void displayPlayersByTeamId(TeamModel allTeams[], PlayerModel allPlayers[], int teamId, int teamModelCount)
{
    int foundTeamIndex = searchTeamIndexById(allTeams, teamId, teamModelCount);

    if(foundTeamIndex == -1)
    {
        printf("Team not found.\n");
        return;
    }

    printf("Players of Team %s:\n", allTeams[foundTeamIndex].name);
    printf("================================================================================================================\n");
    printf("%-5s %-20s %-15s %-10s %-10s %-10s %-10s %-10s %-10s\n",
           "ID", "Name", "Role", "Runs", "Avg", "SR", "Wkts", "ER", "Perf.Index");
    printf("================================================================================================================\n");

    for(int teamPlayerIndex = 0; teamPlayerIndex < allTeams[foundTeamIndex].totalPlayers; teamPlayerIndex++)
    {
        int playerIndex = allTeams[foundTeamIndex].playerIndexList[teamPlayerIndex];

        printf("%-5d %-20s %-15s %-10d %-10.2f %-10.2f %-10d %-10.2f %-10.2f\n\n",
               allPlayers[playerIndex].playerId,
               allPlayers[playerIndex].name,
               allPlayers[playerIndex].role,
               allPlayers[playerIndex].totalRuns,
               allPlayers[playerIndex].battingAverage,
               allPlayers[playerIndex].strikeRate,
               allPlayers[playerIndex].wickets,
               allPlayers[playerIndex].economyRate,
               allPlayers[playerIndex].performanceIndex);
    }

    printf("================================================================================================================\n");
    printf("Total Players: %d\n",allTeams[foundTeamIndex].totalPlayers);
    printf("Average Batting Strike Rate: %0.2f\n\n",allTeams[foundTeamIndex].averageBattingStrikeRate);
}

void mergeTeamSegmentsByStrikeRate(TeamModel allTeams[], int startIndex, int middleIndex, int endIndex)
{
    int leftArraySize = middleIndex - startIndex + 1;
    int rightArraySize = endIndex - middleIndex;

    TeamModel leftTeamArray[leftArraySize];
    TeamModel rightTeamArray[rightArraySize];

    for(int leftTeamCopyIndex = 0; leftTeamCopyIndex < leftArraySize; leftTeamCopyIndex++)
    {
        leftTeamArray[leftTeamCopyIndex] = allTeams[startIndex + leftTeamCopyIndex];        
    }

    for(int rightTeamCopyIndex = 0; rightTeamCopyIndex < rightArraySize; rightTeamCopyIndex++)
    {
        rightTeamArray[rightTeamCopyIndex] = allTeams[middleIndex + 1 + rightTeamCopyIndex];
    }

    int leftTeamPosition = 0;
    int rightTeamPosition = 0;
    int writeTeamPosition = startIndex;

    while(leftTeamPosition < leftArraySize && rightTeamPosition < rightArraySize)
    {
        if(leftTeamArray[leftTeamPosition].averageBattingStrikeRate >= rightTeamArray[rightTeamPosition].averageBattingStrikeRate)
        {
            allTeams[writeTeamPosition++] = leftTeamArray[leftTeamPosition++];            
        }
        else
        {
            allTeams[writeTeamPosition++] = rightTeamArray[rightTeamPosition++];
        }
    }

    while(leftTeamPosition < leftArraySize)
    {
        allTeams[writeTeamPosition++] = leftTeamArray[leftTeamPosition++];
    }

    while(rightTeamPosition < rightArraySize)
    {
        allTeams[writeTeamPosition++] = rightTeamArray[rightTeamPosition++];
    }
}

void sortTeamsByStrikeRate(TeamModel allTeams[], int startIndex, int endIndex)
{
    if(startIndex < endIndex)
    {
        int midIndex = startIndex + (endIndex - startIndex) / 2;

        sortTeamsByStrikeRate(allTeams, startIndex, midIndex);
        sortTeamsByStrikeRate(allTeams, midIndex + 1, endIndex);

        mergeTeamSegmentsByStrikeRate(allTeams, startIndex, midIndex, endIndex);
    }
}

void displayTeamsByAverageStrikeRate(TeamModel allTeams[], int teamModelCount)
{
    TeamModel sortedTeams[teamModelCount];

    for(int teamCopyIndex = 0; teamCopyIndex < teamModelCount; teamCopyIndex++)
    {
        sortedTeams[teamCopyIndex] = allTeams[teamCopyIndex];
    }

    sortTeamsByStrikeRate(sortedTeams, 0, teamModelCount - 1);

    printf("\nTeams Sorted by Average Batting Strike Rate\n\n");
    printf("=====================================================================\n");
    printf("%-10s %-25s %-15s %-10s\n", "ID", "Team Name", "Avg Bat SR", "Players");
    printf("=====================================================================\n");

    for(int sortedTeamIndex = 0; sortedTeamIndex < teamModelCount; sortedTeamIndex++)
    {
        printf("%-10d %-25s %-15.2f %-10d\n",
               sortedTeams[sortedTeamIndex].teamId,
               sortedTeams[sortedTeamIndex].name,
               sortedTeams[sortedTeamIndex].averageBattingStrikeRate,
               sortedTeams[sortedTeamIndex].totalPlayers);
    }

    printf("=====================================================================\n");
}

void displayTopKPlayersByRole(TeamModel allTeams[], PlayerModel allPlayers[], int teamModelCount)
{
    int inputTeamId;
    int roleChoice;
    int inputNoOfPlayers;

    printf("\nEnter Team ID: ");
    inputTeamId = getValidatedInt();

    int foundTeamIndex = searchTeamIndexById(allTeams, inputTeamId, teamModelCount);

    if(foundTeamIndex == -1)
    {
        printf("Error: Invalid Team ID. Team does not exist.\n");
        return;
    }

    printf("Enter Role (1-Batsman, 2-Bowler, 3-All-rounder): ");
    roleChoice = getValidatedInt();

    if(roleChoice < 1 || roleChoice > 3)
    {
        printf("Error: Invalid role choice.\n");
        return;
    }

    printf("Enter number of players: ");
    inputNoOfPlayers = getValidatedInt();

    int *selectedRoleList = NULL;
    int selectedRoleCount = 0;
    char roleDisplayName[MAX_CHARACTER_LENGTH];

    if(roleChoice == 1)
    {
        selectedRoleList = allTeams[foundTeamIndex].batsmanList;
        selectedRoleCount = allTeams[foundTeamIndex].batsmanCount;
        strcpy(roleDisplayName, "Batsmen");
    }
    else if(roleChoice == 2)
    {
        selectedRoleList = allTeams[foundTeamIndex].bowlerList;
        selectedRoleCount = allTeams[foundTeamIndex].bowlerCount;
        strcpy(roleDisplayName, "Bowlers");
    }
    else
    {
        selectedRoleList = allTeams[foundTeamIndex].allrounderList;
        selectedRoleCount = allTeams[foundTeamIndex].allrounderCount;
        strcpy(roleDisplayName, "All-rounders");
    }

    if(selectedRoleCount == 0)
    {
        printf("No players of this role in the team.\n");
        return;
    }

    if(inputNoOfPlayers > selectedRoleCount)
    {
        printf("Error: Only %d %s present in this team.\n\n",selectedRoleCount, roleDisplayName);
        return;
    }

    printf("\nTop %d %s of Team %s:\n",
           inputNoOfPlayers, roleDisplayName, allTeams[foundTeamIndex].name);

    printf("=========================================================================================================\n");
    printf("%-5s %-20s %-15s %-10s %-10s %-10s %-10s %-10s %-10s\n",
           "ID", "Name", "Role", "Runs", "Avg", "SR", "Wkts", "ER", "Perf.Index");
    printf("=========================================================================================================\n");

    for(int resultIndex = 0; resultIndex < inputNoOfPlayers; resultIndex++)
    {
        int playerIndex = selectedRoleList[resultIndex];

        printf("%-5d %-20s %-15s %-10d %-10.2f %-10.2f %-10d %-10.2f %-10.2f\n\n",
            allPlayers[playerIndex].playerId,
            allPlayers[playerIndex].name,
            allPlayers[playerIndex].role,
            allPlayers[playerIndex].totalRuns,
            allPlayers[playerIndex].battingAverage,
            allPlayers[playerIndex].strikeRate,
            allPlayers[playerIndex].wickets,
            allPlayers[playerIndex].economyRate,
            allPlayers[playerIndex].performanceIndex
        );
    }
}

void swapHeapNodes(HeapNode *node1, HeapNode *node2)
{
    HeapNode temperoryNode = *node1;
    *node1 = *node2;
    *node2 = temperoryNode;
}

void heapifyDown(HeapNode heap[], int heapSize, int currentIndex, PlayerModel allPlayers[])
{
    int largestIndex = currentIndex;
    int leftChildIndex = 2 * currentIndex + 1;
    int rightChildIndex = 2 * currentIndex + 2;

    if(leftChildIndex < heapSize &&
       allPlayers[heap[leftChildIndex].playerIndex].performanceIndex >
       allPlayers[heap[largestIndex].playerIndex].performanceIndex)
    {
        largestIndex = leftChildIndex;
    }

    if(rightChildIndex < heapSize &&
       allPlayers[heap[rightChildIndex].playerIndex].performanceIndex >
       allPlayers[heap[largestIndex].playerIndex].performanceIndex)
    {
        largestIndex = rightChildIndex;
    }

    if(largestIndex != currentIndex)
    {
        swapHeapNodes(&heap[currentIndex], &heap[largestIndex]);
        heapifyDown(heap, heapSize, largestIndex, allPlayers);
    }
}

void insertIntoHeap(HeapNode heap[], int *heapSize, HeapNode newNode, PlayerModel allPlayers[])
{
    int insertIndex = *heapSize;
    int currentIndex = insertIndex;

    heap[insertIndex] = newNode;
    (*heapSize)++;

    while(currentIndex > 0)
    {
        int parentIndex = (currentIndex - 1) / 2;

        if(allPlayers[heap[currentIndex].playerIndex].performanceIndex >
           allPlayers[heap[parentIndex].playerIndex].performanceIndex)
        {
            swapHeapNodes(&heap[currentIndex], &heap[parentIndex]);
            currentIndex = parentIndex;
        }
        else
        {
            break;
        }    
    }
}

HeapNode popMaxPerformancePlayer(HeapNode heap[], int *heapSize, PlayerModel allPlayers[])
{
    HeapNode root = heap[0];

    heap[0] = heap[*heapSize - 1];
    (*heapSize)--;

    heapifyDown(heap, *heapSize, 0, allPlayers);

    return root;
}

void displayPlayersOfAllTeamsByRole(TeamModel allTeams[], PlayerModel allPlayers[], int roleChoice, int teamModelCount)
{
    char roleName[MAX_CHARACTER_LENGTH];

    if(roleChoice == 1)
    {
        strcpy(roleName, "Batsmen");
    }
    else if(roleChoice == 2)
    {
        strcpy(roleName, "Bowlers");
    }
    else
    {
        strcpy(roleName, "All-rounders");
    }

    int totalPlayersOfRole = 0;
    int initialHeapCapacity = teamModelCount;

    HeapNode *heap = (HeapNode *)malloc(initialHeapCapacity * sizeof(HeapNode));

    if(heap == NULL)
    {
        printf("Memory allocation failed.\n");
        return;
    }

    int heapSize = 0;

    for(int teamIndex = 0; teamIndex < teamModelCount; teamIndex++)
    {
        int *roleListPointer = NULL;
        int roleCountForTeam = 0;

        if(roleChoice == 1)
        {
            roleListPointer = allTeams[teamIndex].batsmanList;
            roleCountForTeam = allTeams[teamIndex].batsmanCount;
        }
        else if(roleChoice == 2)
        {
            roleListPointer = allTeams[teamIndex].bowlerList;
            roleCountForTeam = allTeams[teamIndex].bowlerCount;
        }
        else
        {
            roleListPointer = allTeams[teamIndex].allrounderList;
            roleCountForTeam = allTeams[teamIndex].allrounderCount;
        }

        if(roleCountForTeam > 0)
        {
            totalPlayersOfRole += roleCountForTeam;
            HeapNode newNode;
            newNode.playerIndex = roleListPointer[0];
            newNode.teamIndex = teamIndex;
            newNode.positionInList = 0;

            insertIntoHeap(heap, &heapSize, newNode, allPlayers);
        }
    }

    if(totalPlayersOfRole == 0)
    {
        printf("No %s found across any team.\n", roleName);
        free(heap);

        return;
    }

    printf("\nAll %s Across All Teams:\n", roleName);
    printf("==================================================================================================================\n");
    printf("%-5s %-20s %-15s %-15s %-10s %-10s %-10s %-10s %-10s\n",
           "ID", "Name", "Team", "Role", "Runs", "Avg", "SR", "Wkts", "Perf.Index");
    printf("==================================================================================================================\n");

    while(heapSize > 0)
    {
        HeapNode topNode = popMaxPerformancePlayer(heap, &heapSize, allPlayers);
        int playerIndex = topNode.playerIndex;
        int teamIndex = topNode.teamIndex;
        int nextPosition = topNode.positionInList + 1;

        printf("%-5d %-20s %-15s %-15s %-10d %-10.2f %-10.2f %-10d %-10.2f\n",
            allPlayers[playerIndex].playerId,
            allPlayers[playerIndex].name,
            allTeams[teamIndex].name,
            allPlayers[playerIndex].role,
            allPlayers[playerIndex].totalRuns,
            allPlayers[playerIndex].battingAverage,
            allPlayers[playerIndex].strikeRate,
            allPlayers[playerIndex].wickets,
            allPlayers[playerIndex].performanceIndex
        );

        int *roleListPointer;
        int roleCountForTeam;

        if(roleChoice == 1)
        {
            roleListPointer = allTeams[teamIndex].batsmanList;
            roleCountForTeam = allTeams[teamIndex].batsmanCount;
        }
        else if(roleChoice == 2)
        {
            roleListPointer = allTeams[teamIndex].bowlerList;
            roleCountForTeam = allTeams[teamIndex].bowlerCount;
        }
        else
        {
            roleListPointer = allTeams[teamIndex].allrounderList;
            roleCountForTeam = allTeams[teamIndex].allrounderCount;
        }

        if(nextPosition < roleCountForTeam)
        {
            HeapNode newNode;
            newNode.playerIndex = roleListPointer[nextPosition];
            newNode.teamIndex = teamIndex;
            newNode.positionInList = nextPosition;

            insertIntoHeap(heap, &heapSize, newNode, allPlayers);
        }
    }

    printf("==================================================================================================================\n");
    free(heap);
}


int validateIntegerInput(char *inputString)
{
    int charIndex = 0;

    if (inputString[0] == '\0' || inputString[0] == '-' || inputString[0] == '+')
    {
        return 0;
    }

    while (inputString[charIndex] != '\0')
    {
        if (inputString[charIndex] < '0' || inputString[charIndex] > '9')
        {
            return 0;
        }

        charIndex++;
    }

    return 1;
}

int getValidatedInt()
{
    char inputBuffer[MAX_CHARACTER_LENGTH];

    while (1)
    {
        fgets(inputBuffer, sizeof(inputBuffer), stdin);

        int index = 0;

        while (inputBuffer[index] != '\n' && inputBuffer[index] != '\0')
        {
            index++;
        }

        inputBuffer[index] = '\0';

        if (validateIntegerInput(inputBuffer))
        {
            return (short int)atoi(inputBuffer);
        }

        printf("Invalid input! Enter a valid integer: ");
    }
}
