#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char* argv[]){
    if (argc < 2){
        perror("Missing filename");
        exit(EXIT_FAILURE);
    }
    int file = open(argv[1], O_RDWR);
    if (file == -1){
        perror("failed open");
        exit(EXIT_FAILURE);
    }

    struct flock info;
    info.l_len = 0;
    info.l_start = 0;
    info.l_whence = SEEK_SET;
    info.l_type = F_RDLCK;

    if (fcntl(file, F_SETLK, &info)){
        perror("failed fcntl");
        exit(EXIT_FAILURE);
    }
    char* command = malloc(strlen(argv[1]) + 4);
    if (command == NULL){
        perror("failed malloc");
        exit(EXIT_FAILURE);
    }
    if (sprintf(command, "vi %s", argv[1])<0){
        perror("failed sprintf");
        exit(EXIT_FAILURE);
    }

    if (system(command) == -1){
        perror("failed system");
        exit(EXIT_FAILURE);
    }

    info.l_type = F_UNLCK;
    if (fcntl(file, F_SETLK, &info)){
        perror("failed fcntl");
        exit(EXIT_FAILURE);
    }

    free(command);
    close(file);

    exit(EXIT_SUCCESS);
}