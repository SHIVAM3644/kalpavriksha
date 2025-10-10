#include <stdio.h>
#include <string.h>
#define max_name_length 50
#define max_input_scan 250
	
void showStudentPerformance(short int StudentRoll, char studentName[], int marks[]);
int calculateTotalMarks(int marks1,int marks2,int marks3);
float calculateAverageMarks(int marks1,int marks2,int marks3);
char calculateGrade(float averageMarks);
char* gradeToStars(char grade);

	
struct Student
{ 
    short int rollNo;
    char nameOfStudent[max_name_length];
    int marksOfThreeSubject[3];
	 	
};

int main()
{   
    short int totalNumberOfStudents;
       
    printf("Enter Numbers Of Student\n");
    scanf("%d",&totalNumberOfStudents);
    getchar();
        
    struct Student s[totalNumberOfStudents];

    for (int i = 0; i < totalNumberOfStudents; i++)
    {
        char line[max_input_scan];

        fgets(line, sizeof(line), stdin);

        line[strcspn(line, "\n")] = '\0';

        sscanf(line, "%hd %[^\n0123456789] %d %d %d",
            &s[i].rollNo,
            s[i].nameOfStudent,
            &s[i].marksOfThreeSubject[0],
            &s[i].marksOfThreeSubject[1],
	        &s[i].marksOfThreeSubject[2]);
	} 
	    
    for(int i = 0; i < totalNumberOfStudents; i++)
	{
		showStudentPerformance(s[i].rollNo,s[i].nameOfStudent,s[i].marksOfThreeSubject);
	}

	printf("List of Roll Numbers: "); 

    for(int i = 0; i < totalNumberOfStudents; i++)
	{
		printf("%d  ",s[i].rollNo);
	}

     return 0;
    
}

void showStudentPerformance(short int StudentRoll,char nameOfStudent[],int marks[])
{
	
	int totalMarks = calculateTotalMarks(marks[0], marks[1], marks[2]);
    float averageMarks = calculateAverageMarks(marks[0], marks[1], marks[2]);
    
	printf("Roll: %d\n",StudentRoll);
	printf("Name: %s\n",nameOfStudent);
	printf("Total: %d\n",totalMarks);
	printf("Average: %f\n",averageMarks);
	printf("Grade: %c\n",calculateGrade(averageMarks));
	
	if(calculateGrade(averageMarks)!='F')
	{
		printf("Performance: %s\n\n",gradeToStars(calculateGrade(averageMarks)));
	}
	
		
}

int calculateTotalMarks(int marks1,int marks2,int marks3)
{
	return marks1+marks2+marks3;
}

float calculateAverageMarks(int marks1,int marks2,int marks3)
{
	return (marks1+marks2+marks3)/3.0;
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
   switch(grade) {
       case 'A': return "*****";
       case 'B': return "****";
       case 'C': return "***";
       case 'D': return "**";
       default:  return "";
   }
}