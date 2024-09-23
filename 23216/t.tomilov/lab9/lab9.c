#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <wait.h>

void cat(char* filename){
    FILE* file = fopen(filename, "r");
    if (file == NULL){
        perror("Error: can`t open file!");
        exit(-1);
    }
    char sym;
    while ((sym = fgetc(file)) != EOF) {
        putchar(sym);
    }
    putchar('\n');
    fclose(file);
}

int main(int argc, char** argv){
    if (argc > 2 || argc < 2){
        perror("Error: must be only 1 argument: ./lab9 <file>");
        exit(-1);
    }
    printf("Child process starded!\n");
    pid_t child_process = fork();
    if (child_process == -1){
        perror("Error: Cant`t creat child process!");
        exit(-1);
    }
    else if(child_process == 0){
        cat(argv[1]);
    }
    else{
        int status;
        if (wait(&status) == -1){
            perror("Error: failed to wait!");
        }
        printf("Perent process ended!\n");
        exit(0);
    }
    exit(0);
}
