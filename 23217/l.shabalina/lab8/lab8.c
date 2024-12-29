#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/file.h>

#define COMMAND_SIZE 261

int main(int argc, char* argv[]) {

    if (argc != 2) {
        fprintf(stderr, "usage: %s filename\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    const char* filename = argv[1];

    int fd = open(filename, O_RDWR);
    if (fd == -1) {
        perror("opening file fail");
        exit(EXIT_FAILURE);
    }

    struct flock flockptr;
    flockptr.l_type = F_WRLCK;
    flockptr.l_whence = SEEK_SET;
    flockptr.l_start = 0;
    flockptr.l_len = 0; 

    if (fcntl(fd, F_SETLK, &flockptr) == -1) {
        perror("locking file fail");
        close(fd);
        exit(EXIT_FAILURE);
    }

    printf("file locked.\n");
    char command[COMMAND_SIZE];
    snprintf(command, sizeof(command), "vi %s", filename);

    if (system(command) == -1) {
        perror("system command fail");
        close(fd);
        exit(EXIT_FAILURE);
    }

    flockptr.l_type = F_UNLCK;
    if (fcntl(fd, F_SETLK, &flockptr) == -1) {
        perror("unlocking file fail");
        close(fd);
        exit(EXIT_FAILURE);
    }

    close(fd);
    exit(EXIT_SUCCESS);
}

