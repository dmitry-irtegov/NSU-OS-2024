#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int main() {
    int fd = open("./file.txt", O_RDWR);
    if (fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    struct flock fl;
    fl.l_type   = F_WRLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start  = 0;
    fl.l_len    = 0;

    if (fcntl(fd, F_SETLK, &fl) == -1) {
        perror("fcntl");
        close(fd);
        exit(EXIT_FAILURE);
    }

    int is_success = 1;
    if (system("nano ./file.txt") == -1) {
        perror("system");
        is_success = 0;
    }

    fl.l_type = F_UNLCK;
    if (fcntl(fd, F_SETLK, &fl) < 0) {
        perror("fcntl");
        is_success = 0;
    }

    close(fd);
    exit(is_success ? EXIT_SUCCESS : EXIT_FAILURE);
}
