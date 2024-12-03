#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

int main(int argc, char** argv){
    if (argc != 2){
        fprintf(stderr, "Wrong format: try %s <file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    struct flock lock;

    char* nano = malloc(strlen("nano ") + strlen(argv[1]) + 1);
    if (!nano){
        perror("ERROR: Memory allocation failed!");
        exit(EXIT_FAILURE);
    }

    if (snprintf(nano, strlen("nano ") + strlen(argv[1]) + 1, "nano %s", argv[1]) < 0){
        perror("ERROR: Failed in sprintf!");
        free(nano);
        exit(EXIT_FAILURE);
    }

    int fd = open(argv[1], O_RDWR);
    if (fd == -1){
        perror("ERROR: File doesn't exist or access denied!");
        exit(EXIT_FAILURE);
    }

    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;

    if (fcntl(fd, F_SETLKW, &lock) == -1){
        perror("ERROR: Failed to lock the file!");
        close(fd);
        exit(EXIT_FAILURE);
    }

    if (system(nano) == -1){
        perror("ERROR: Failed to execute nano!");
        lock.l_type = F_UNLCK;
        fcntl(fd, F_SETLK, &lock);
        free(nano);
        close(fd);
        exit(EXIT_FAILURE);
    }

    lock.l_type = F_UNLCK;
    if (fcntl(fd, F_SETLK, &lock) == -1){
        perror("ERROR: Failed to unlock the file!");
        free(nano);
        close(fd);
        exit(EXIT_FAILURE);
    }

    free(nano);
    close(fd);
    return EXIT_SUCCESS;
}
