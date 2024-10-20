#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#define MAX_SIZE 256
void enter_filename(char* filename);
void print_ID();
int set_alike_ID(uid_t ID);
void open_file(char* filename);

int main() {
    char filename[MAX_SIZE] = "file.txt";
    //first time
    print_ID();
    open_file(filename);

    if (setuid(getuid()) == 0) {
        printf("Everything is fine! ID changed correctly!\n");
    } else {
        perror("You can't change ID.\n");
        exit(EXIT_SUCCESS);
    }
    //second time
    print_ID();
    open_file(filename);
}

void print_ID() {
    uid_t real_ID = getuid();
    printf("Your real ID is: %d\n", real_ID);
    printf("Your effective ID is: %d\n", geteuid());
}

void open_file(char* filename) {
    FILE* in = fopen(filename, "r");
    if (in == NULL) {
        perror("Error: ");
        exit(0);
    } else {
        printf("File is opened correctly!");
        fclose(in);
    }
}

void enter_filename(char* filename) { 
    printf("Enter filename: ");
    fgets(filename, MAX_SIZE, stdin);
    size_t len = strlen(filename);
    printf("filename[len - 1] %c", filename[len - 1]);
    if (len > 0 && filename[len - 1] == '\n') {
            filename[len - 1] = '\0';
    }
    if (strlen(filename) == MAX_SIZE - 1 && filename[MAX_SIZE - 2] != '\n') {
        perror("You can't find file with such long file_name!");
        exit(EXIT_FAILURE);
    } else {
        printf("name is correct\n");
        open_file(filename);
    }

}
