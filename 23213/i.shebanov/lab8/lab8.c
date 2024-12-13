#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

int main(int argc, char * argv[]) {
    if (argc == 1) {
        fprintf(stderr, "Error: No file is specified\n");
        exit(1);
    }

    int fd = open(argv[1], O_RDWR);
    if (fd == -1) {
        perror("Could not open the file");
        exit(1);
    }

    struct flock flck;
    flck.l_type = F_WRLCK;
    flck.l_whence = SEEK_SET;
    flck.l_start = 0;
    flck.l_len = 0;
    flck.l_pid = getpid();

    if (fcntl(fd, F_SETLK, &flck) == -1) {
        perror("Failed to set lock");
        exit(1);
    }

    char command[128];
    snprintf(command, sizeof(command), "vim %s", argv[1]);

    if (system(command) == -1) {
        perror("System command failed");
        exit(1);
    }

    
    return 0;
}
