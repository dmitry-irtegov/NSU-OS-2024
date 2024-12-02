#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

int main(int argc, char** argv){
    if (argc != 2){
        perror("you have to give file name");
        exit(EXIT_FAILURE);
    }
    struct flock lock;
    int file = open(argv[1], O_RDWR | O_CREAT, 0777);
    if (file == -1){
        perror("problem in file open");
        exit(EXIT_FAILURE);
    }
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    if (fcntl(file, F_SETLK, &lock) == -1){
        perror("problem in lock");
        close(file);
        exit(EXIT_FAILURE);
    }
    char* cmd = malloc(strlen("nano ") + strlen(argv[1]) + 1);
    if (!cmd){
        perror("problem in malloc");
        lock.l_type = F_UNLCK;
        if (fcntl(file, F_SETLK, &lock) == -1){
            perror("problem in unlock");
            close(file);
            exit(EXIT_FAILURE);
        }
        close(file);
        exit(EXIT_FAILURE);
    }
    if (snprintf(cmd, strlen("nano ") + strlen(argv[1]) + 1, "nano %s", argv[1]) < 0){
        perror("problem in snprintf");
        free(cmd);
        lock.l_type = F_UNLCK;
        if (fcntl(file, F_SETLK, &lock) == -1){
            perror("problem in unlock");
            close(file);
            exit(EXIT_FAILURE);
        }
        close(file);
        exit(EXIT_FAILURE);
    }
    if (system(cmd) == -1){
        perror("failed in nano");
        free(cmd);
        lock.l_type = F_UNLCK;
        if (fcntl(file, F_SETLK, &lock) == -1){
            perror("problem in unlock");
            close(file);
            exit(EXIT_FAILURE);
        }
        close(file);
        exit(EXIT_FAILURE);
    }
    lock.l_type = F_UNLCK;
    if (fcntl(file, F_SETLK, &lock) == -1){
        perror("problem in unlock");
        free(cmd);
        lock.l_type = F_UNLCK;
        if (fcntl(file, F_SETLK, &lock) == -1){
            perror("problem in unlock");
            close(file);
            exit(EXIT_FAILURE);
        }
        close(file);
        exit(EXIT_FAILURE);
    }
    free(cmd);
    close(file);
    exit(EXIT_SUCCESS);
}