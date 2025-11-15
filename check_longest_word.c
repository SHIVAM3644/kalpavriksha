#include <stdio.h>

#define MAXIMUM_STRING_SIZE 50

int calculateStringLength(char *pointerToString)
{
    int stringLength = 0;

    while (*pointerToString != '\0') 
    {   
        stringLength++;
        pointerToString++;
    }

    return stringLength;
}

int main()
{
    int noOfWords;

    printf("Enter number of words: ");
    
    if (scanf("%d", &noOfWords) != 1 || noOfWords <= 0) 
    {
        printf("Error: Please enter a valid positive integer for number of words.\n");
        return 1;
    }
     
    getchar();

    char stringArray[noOfWords][MAXIMUM_STRING_SIZE];
    
    printf("Enter words:\n");

    for (int i = 0; i < noOfWords; i++) 
    {   
        fgets(*(stringArray + i), MAXIMUM_STRING_SIZE, stdin);
        char *pointerToString = *(stringArray + i);
        
        //this is used to check the buffer is containing \n at end
        while (*pointerToString != '\0')
        {
            pointerToString++;
        } 

        if (*(pointerToString - 1) == '\n')
        {
            *(pointerToString - 1) = '\0';
        }
    }
    
    int maximumLengthStringIndex = 0;
    char *pointerToLongestString = *stringArray;

    for (int i = 1; i < noOfWords; i++) 
    {
        char *pointerToString = *(stringArray + i);
        
        if (calculateStringLength(pointerToLongestString) < calculateStringLength(pointerToString))
        {
            pointerToLongestString = pointerToString;
            maximumLengthStringIndex = i;            
        }       
    }

    printf("\nThe Longest Word is: %s\n", *(stringArray + maximumLengthStringIndex));
    printf("Length: %d\n", calculateStringLength(*(stringArray + maximumLengthStringIndex)));

    return 0;
}
