#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

void print_uids() {
    uid_t real_uid = getuid();
    uid_t effective_uid = geteuid();
    printf("Real UID: %d, Effective UID: %d\n", real_uid, effective_uid);
}

int main() {
    print_uids();

    int file = open("file.txt", O_RDONLY);
    if (file == -1) {
        perror("Error opening file");
    } else {
        printf("File opened successfully.\n");
        close(file);
    }

    if (setuid(getuid()) == -1) {
        perror("setuid failed");
        exit(EXIT_FAILURE);
    }

    print_uids();

    file = open("file.txt", O_RDONLY);
    if (file == -1) {
        perror("Error opening file");
    } else {
        printf("File opened successfully.\n");
        close(file); 
    }

    return 0;
}
 