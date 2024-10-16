#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#define MAX_SIZE 257
void enter_filename(char* filename);
void print_ID(uid_t efctv_ID);
int set_alike_ID(uid_t ID);
void open_file(char* filename);

int main() {
    char filename[MAX_SIZE];
    uid_t efctv_ID = geteuid();
    
    //first time
    print_ID(efctv_ID);
    enter_filename(filename);
    open_file(filename);

    if (setuid(getuid()) == 0) {
        printf("Everything is fine! ID changed correctly!\n");
    } else {
        perror("You can't change ID.");
        exit(0);
    }
    
    //second time
    print_ID(efctv_ID);
    enter_filename(filename);
    open_file(filename);
}

void print_ID(uid_t efctv_ID) {
    uid_t real_ID = getuid();
    printf("Your real ID is: %d\n", real_ID);
    printf("Your effective ID is: %d\n", efctv_ID);
}

int set_alike_ID(uid_t ID) {
    return setuid(ID);
}

void open_file(char* filename) {
    FILE* in = fopen(filename, "r");
    if (in == NULL) {
        perror("Error: ");
    } else {
        printf("FIle is opened correctly!");
        fclose(in);
    }
}
void enter_filename(char* filename) { 
    printf("Enter filename: ");
    scanf("%s", filename);
}
