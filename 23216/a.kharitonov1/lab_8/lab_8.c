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
    int fd = open(argv[1], O_RDWR | O_CREATE);
    if (fd == -1){
        perror("problem in file open");
        exit(EXIT_FAILURE);
    }
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    if (fcntl(fd, F_SETLKW, &lock) == -1){
        perror("problem in lock");
        close(fd);
        exit(EXIT_FAILURE);
    }
    char* nano = malloc(strlen("nano ") + strlen(argv[1]) + 1);
    if (!nano){
        perror("problem in malloc");
        close(fd);
        exit(EXIT_FAILURE);
    }
    if (snprintf(nano, strlen("nano ") + strlen(argv[1]) + 1, "nano %s", argv[1]) < 0){
        perror("problem in snprintf");
        free(nano);
        close(fd);
        exit(EXIT_FAILURE);
    }
    if (system(nano) == -1){
        perror("failed in nano");
        free(nano);
        close(fd);
        exit(EXIT_FAILURE);
    }
    lock.l_type = F_UNLCK;
    if (fcntl(fd, F_SETLK, &lock) == -1){
        perror("problem in unlock");
        free(nano);
        close(fd);
        exit(EXIT_FAILURE);
    }
    free(nano);
    close(fd);
    exit(EXIT_SUCCESS);
}