#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#define MAX_SIZE 256
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
        exit(EXIT_FAILURE);
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
        exit(EXIT_FAILURE);
    } else {
        printf("File is opened correctly!\n\n");
        fclose(in);
    }
}

