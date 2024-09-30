#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

void openFile(char* fileName){
    FILE* file = fopen(fileName, "r");
    if (file == NULL){
        perror("   ERROR: file didn`t opened!");
        exit(EXIT_FAILURE);
    }
    else{
        printf("   File %s opened!\n", fileName);
        fclose(file);
    }
}

int main(int argc, char** argv){;
    if (!(argc == 2)){
        perror("ERROR: wrong format of start. Try ./lab3 <file>");
        exit(EXIT_FAILURE);
    }
    printf("1. Real UID: %d\n   Effective UID: %d\n", getuid(), geteuid());
    openFile(argv[1]);
    if (setuid(getuid())){
        perror("ERROR: failed to setuid!");
        exit(EXIT_FAILURE);
    }
    printf("2. Real UID: %d\n   Effective UID: %d\n", getuid(), geteuid());
    openFile(argv[1]);
    exit(EXIT_SUCCESS);
}