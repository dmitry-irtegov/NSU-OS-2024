#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

void messageAboutOpenAndClose(FILE* file, char* message) {
    puts(message);
    fclose(file);
}

void printErrorAndExit(char* message){
    perror(message);
    exit(2);
}

void tryToOpen(FILE* file, char* filename, char* message){
    printf("uid = %d and euid = %d\n", getuid(), geteuid());
    file = fopen(filename, "r");

    if (file == NULL) {
        perror(filename);
        exit(2);
    }
    else {
        puts(message);
        fclose(file);
    }
}

int main(int argc, char *argv[])
{
    FILE *file;

    if (argc < 2){
        puts("missing filename");
        exit(1);
    }

    tryToOpen(file, argv[1], "first open!");

    if (setuid(getuid())){
        perror("failed to use setuid");
    }
    
    tryToOpen(file, argv[1], "second open!");
    exit(0);
}