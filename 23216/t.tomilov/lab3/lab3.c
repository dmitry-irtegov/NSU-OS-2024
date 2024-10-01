#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

char openFile(char* fileName){
    FILE* file = fopen(fileName, "r");
    if (file == NULL){
        perror("   ERROR: file didn`t opened!");
        return '0';
    }
    printf("   File %s opened!\n", fileName);
    fclose(file);
    return '1';
}

int main(int argc, char** argv){;
    if (!(argc == 2)){
        perror("ERROR: wrong format of start. Try ./lab3 <file>");
        exit(EXIT_FAILURE);
    }
    printf("1. Real UID: %d\n   Effective UID: %d\n", getuid(), geteuid());
    if (openFile(argv[1]) == '0') exit(EXIT_FAILURE);
    if (setuid(getuid())){
        perror("ERROR: failed to setuid!");
        exit(EXIT_FAILURE);
    }
    printf("2. Real UID: %d\n   Effective UID: %d\n", getuid(), geteuid());
    if (openFile(argv[1]) == '0') exit(EXIT_FAILURE);
    exit(EXIT_SUCCESS);
}