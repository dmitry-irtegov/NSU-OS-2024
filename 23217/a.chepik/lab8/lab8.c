#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("File name not found in argv.\n");
        exit(-1);
    }

    const char* filename = argv[1];

    int file_descriptor = open(filename, O_RDWR);
    if (file_descriptor == -1) {
        printf("Error opening file.\n");
        exit(-1);
    }

    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;

    if (fcntl(file_descriptor, F_SETLK, &lock) == -1) {
        printf("Error locking file.\n");
        close(file_descriptor);
        exit(-1);
    }

    printf("File locked.\n");

    char command[8192];
    snprintf(command, sizeof(command), "vi %s\n", filename);

    if (system(command) == -1) {
        printf("System command: fail.\n");
        close(file_descriptor);
        exit(-1);
    }

    lock.l_type = F_UNLCK;
    
    if (fcntl(file_descriptor, F_SETLK, &lock) == -1) {
        printf("Error unlocking file.\n");
        close(file_descriptor);
        exit(-1);
    }

    close(file_descriptor);

   exit(0);
}
