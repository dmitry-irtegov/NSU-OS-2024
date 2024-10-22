#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

void exitProgram(int status, char* message) {
    if (message != NULL) {
        perror(message);
    }
    exit(status);
}

int main(int argc, char* argv[]){
    if (argc < 2){
        fprintf(stderr, "Missing filename\n");
        exitProgram(EXIT_FAILURE, NULL);
    }
    int file = open(argv[1], O_RDWR);
    if (file == -1){
        exitProgram(EXIT_FAILURE, "failed open");
    }

    struct flock info;
    info.l_len = 0;
    info.l_start = 0;
    info.l_whence = SEEK_SET;
    info.l_type = F_WRLCK;

    if (fcntl(file, F_SETLK, &info)){
        exitProgram(EXIT_FAILURE, "failed fcntl");
    }
    char* command = malloc(strlen(argv[1]) + 4);
    if (command == NULL){
        exitProgram(EXIT_FAILURE, "failed malloc");
    }
    if (sprintf(command, "vi %s", argv[1])<0){
        exitProgram(EXIT_FAILURE, "failed sprintf");
    }

    if (system(command) == -1){
        exitProgram(EXIT_FAILURE, "failed system");
    }

    info.l_type = F_UNLCK;
    if (fcntl(file, F_SETLK, &info)){
        exitProgram(EXIT_FAILURE, "failed fcntl");
    }

    free(command);
    close(file);

    exitProgram(EXIT_SUCCESS, NULL);
}