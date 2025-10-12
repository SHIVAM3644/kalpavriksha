#include <stdio.h>
#include <string.h>
#define max_name_length 50
#define max_input_scan 250
#define SUBJECT_COUNT 3
	
struct Student
{ 
    short int rollNo;
    char nameOfStudent[max_name_length];
    int marksOfThreeSubject[SUBJECT_COUNT];
	 	
};

void showStudentPerformance(struct Student studentInstance[],int totalNumberOfStudents);

int calculateTotalMarks(int marksOfStudent[], int totalNumberOfSubjects);
 
float calculateAverageMarks(int marksOfStudent[], int totalNumberOfSubjects);

char calculateGrade(float averageMarks);

char* gradeToStars(char grade);

void printListOfRollNo(struct Student studentInstance[], int index, int totalStudents);

int main()
{   
    short int totalNumberOfStudents;
       
    printf("Enter Numbers Of Student\n");
    
    scanf("%d",&totalNumberOfStudents);
    
    getchar();
        
    struct Student studentInstance[totalNumberOfStudents];

    for (int i = 0; i < totalNumberOfStudents; i++)
    {
        char line[max_input_scan];
        
        fgets(line, sizeof(line), stdin);
        
        line[strcspn(line, "\n")] = '\0';


        sscanf(line, "%hd %[^\n0123456789] %d %d %d",
            &studentInstance[i].rollNo,
            studentInstance[i].nameOfStudent,
            &studentInstance[i].marksOfThreeSubject[0],
            &studentInstance[i].marksOfThreeSubject[1],
	        &studentInstance[i].marksOfThreeSubject[2]);
	} 
	    
    showStudentPerformance(studentInstance,totalNumberOfStudents);
    
    printf("List of Roll Numbers: ");
    
    printListOfRollNo(studentInstance, 0, totalNumberOfStudents);
    
    printf("\n");

     return 0;
    
}

void showStudentPerformance( struct Student studentInstance[],int totalNumberOfStudents )
{   
    printf("\n");
    
	for(int i=0 ; i<totalNumberOfStudents;i++)
	{
		int totalMarks = calculateTotalMarks(studentInstance[i].marksOfThreeSubject,SUBJECT_COUNT);
		
        float averageMarks = calculateAverageMarks(studentInstance[i].marksOfThreeSubject,SUBJECT_COUNT);
    
	    printf("Roll: %d\n",studentInstance[i].rollNo);
	    
	    printf("Name: %s\n",studentInstance[i].nameOfStudent);
	    
	    printf("Total: %d\n",totalMarks);
	    
	    printf("Average: %.2f\n",averageMarks);
	    
	    printf("Grade: %c\n",calculateGrade(averageMarks));
	    
	    if(calculateGrade(averageMarks)=='F')
	    {    
	        printf("\n");
	       	continue;
	    	
	    }else
	    {
	    	printf("Performance: %s\n\n",gradeToStars(calculateGrade(averageMarks)));
	    }
	}
}

int  calculateTotalMarks(int marksOfStudent[], int totalNumberOfSubjects)
{
    int totalMarks = 0;

    for (int i = 0; i < totalNumberOfSubjects; i++)
    {
        totalMarks += marksOfStudent[i];
    }

    return totalMarks;
}

float calculateAverageMarks(int marksOfStudent[], int totalNumberOfSubjects)
{
    return (float)calculateTotalMarks(marksOfStudent, totalNumberOfSubjects) / totalNumberOfSubjects;
}

char calculateGrade(float averageMarks)
{
	if(averageMarks>85.0 && averageMarks<100.0)
	{
		return 'A';
		
	}else if(averageMarks>70.0 && averageMarks<85.0)
	{
		return 'B';
		
	}else if(averageMarks>50.0 && averageMarks<70.0)
	{
		return 'C';
		
	}else if(averageMarks>35.0 && averageMarks<50.0)
	{
		return 'D';
		
	}else 
	{
		return 'F';
	}
}

char* gradeToStars(char grade)
{
    switch (grade)
    {
        case 'A':
        {
            return "*****";
        }

        case 'B':
        {
            return "****";
        }

        case 'C':
        {
            return "***";
        }

        case 'D':
        {
            return "**";
        }

        default:
        {
            return "";
        }
    }
}

void printListOfRollNo(struct Student studentInstance[], int index, int totalStudents) 
{
    if(index >= totalStudents)
	{
	     return;	
	}
    printf("%d ", studentInstance[index].rollNo);
    
    printListOfRollNo(studentInstance, index + 1, totalStudents);
}

