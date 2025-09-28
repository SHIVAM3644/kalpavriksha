#include <stdio.h>
#include <string.h>

struct Student {
    int id;
    char name[50];
    char className[20];
    char address[100];
};

int main() {
    struct Student s;
    FILE *fp;
    int existingId, found = 0;
    char line[200];

    printf("Enter Student ID: ");
    scanf("%d", &s.id);
    getchar();
 

    printf("Enter Name: ");
    fgets(s.name, sizeof(s.name), stdin);

    printf("Enter Gender: ");
    fgets(s.className, sizeof(s.className), stdin);
   

    printf("Enter Address: ");
    fgets(s.address, sizeof(s.address), stdin);
   


    fp = fopen("students.txt", "r");
    if (fp != NULL) {
        while (fgets(line, sizeof(line), fp)) {
       
            if (sscanf(line, "ID: %d", &existingId) == 1) {
                if (existingId == s.id) {
                    found = 1;
                    break;
                }
            }
        }
        fclose(fp);
    }

    if (found) {
        printf(" Record with ID %d already exists. Not adding.\n", s.id);
        return 0;
    }

    fp = fopen("students.txt", "a");
    if (fp == NULL) {
        printf("Error opening file!\n");
        return 1;
    }

    fprintf(fp, "\nID: %d\nName: %s\nGender: %s\nAddress: %s\n",
            s.id, s.name, s.className, s.address);

    fclose(fp);

    printf(" New student record added successfully.\n");

    return 0;
}
