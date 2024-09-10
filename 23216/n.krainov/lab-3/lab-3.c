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

int main(int argc, char *argv[])
{
    FILE *file;

    if (argc < 2){
        puts("missing filename");
        exit(1);
    }

    printf("uid = %d and euid = %d\n", getuid(), geteuid());
    file = fopen(argv[1], "r");

    if (file == NULL) {
        printErrorAndExit(argv[0]);
    }
    else {
        messageAboutOpenAndClose(file, "first open!\n");
    }

    if (setuid(getuid())){
        perror("failed to use setuid");
    }
    
    printf("after setuid: uid = %d, euid = %d\n", getuid(), geteuid() );
    
    file = fopen(argv[1], "r");
    if (file == NULL) {
        printErrorAndExit(argv[0]);
    }
    else {
        messageAboutOpenAndClose(file, "second open\n");
    }
}