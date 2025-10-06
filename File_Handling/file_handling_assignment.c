#include <stdio.h>
#include <string.h>

struct User {
    int id;
    char name[50];
    int age;
};

void addUser();
void deleteUser();
void updateUser();
void readUser();

int main() {

    int choice;

    do {
        printf("\n--- User Record\n");
        printf("1. Add User\n");
        printf("2. Delete User\n");
        printf("3. Update User\n");
        printf("4. Read Record\n");
        printf("5. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);
        getchar();

        switch (choice) {
            case 1: addUser(); break;
            case 2: deleteUser(); break;
            case 3: updateUser(); break;
            case 4: readUser(); break;
            case 5: printf("Exiting...\n"); break;
            default: printf("Invalid choice!\n");
        }
    } while (choice != 5);

    return 0;
}

void addUser() {
    struct User s;
    FILE *fp;
    int existingId, found = 0;
    char line[200];

    printf("Enter User ID: ");
    scanf("%d", &s.id);
    getchar();

    printf("Enter Name: ");
    fgets(s.name, sizeof(s.name), stdin);

    printf("Enter Age: ");
    scanf("%d", &s.age);
    getchar();

    fp = fopen("users.txt", "r");

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

        printf("Record with ID %d already exists\n", s.id);

        return;
    }

    fp = fopen("users.txt", "a");

    if (fp == NULL) {

        printf("Error opening file!\n");
        return;
    }

    fprintf(fp, "ID: %d\nName: %sAge: %d\n\n", s.id, s.name, s.age);

    fclose(fp);

    printf("New User record added successfully.\n");
}

void deleteUser() {
    int deleteId, id;
    char line[200];
    int skip = 0;
    FILE *fp, *temp;

    printf("Enter User ID to delete: ");
    scanf("%d", &deleteId);

    fp = fopen("users.txt", "r");

    if (!fp) { printf("File not found!\n"); return; }

    temp = fopen("temp.txt", "w");

    if (!temp) { printf("Error creating temporary file!\n"); fclose(fp); return; }

    while (fgets(line, sizeof(line), fp)) {
        if (sscanf(line, "ID: %d", &id) == 1) {
            if (id == deleteId) { skip = 3; continue; }
        }
        if (skip > 0) {
            
            skip--;
            continue;
         }
         
        fputs(line, temp);
    }

    fclose(fp);
    fclose(temp);

    remove("users.txt");
    rename("temp.txt", "users.txt");

    printf("Record with ID %d deleted.\n", deleteId);
}

void updateUser() {

    int updateId, id, skip = 0;

    char line[200], name[50];

    int age;

    FILE *fp, *temp;

    printf("Enter User ID to update: ");
    scanf("%d", &updateId);
    getchar();

    fp = fopen("users.txt", "r");

    if (!fp) {
         printf("File not found!\n");
         return;
         }

    temp = fopen("temp.txt", "w");

    if (!temp) {

         printf("Error creating temporary file!\n");
         fclose(fp);
         return;
        
        }

    int found = 0;

    while (fgets(line, sizeof(line), fp)) {
        if (sscanf(line, "ID: %d", &id) == 1) {
            if (id == updateId) {
                found = 1;
                skip = 0;

                printf("Enter new Name: ");
                fgets(name, sizeof(name), stdin);

                printf("Enter new Age: ");
                scanf("%d", &age);
                getchar();

                fprintf(temp, "ID: %d\nName: %sAge: %d\n\n", id, name, age);

                skip = 2;
                continue;
            }
        }

        if (skip > 0) { skip--; continue; }

        fputs(line, temp);
    }

    fclose(fp);
    fclose(temp);

    if (!found) {

        printf("Record with ID %d not found.\n", updateId);
        remove("temp.txt");
        return;
    }

    remove("users.txt");
    rename("temp.txt", "users.txt");

    printf("Record with ID %d updated successfully.\n", updateId);
}

void readUser(){
   
FILE *fp;
char ch;

fp = fopen("users.txt", "r");

    if (fp==NULL) {
         printf("File not found!\n");
         return;
         }

    while((ch=fgetc(fp))!=EOF){

        printf("%c",ch);
    }

    fclose(fp);
}
