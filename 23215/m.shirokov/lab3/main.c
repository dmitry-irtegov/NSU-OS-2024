#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

void print_uids() {
    printf("Real UID: %d, Effective UID: %d\n", getuid(), geteuid());
}

int main() {
    FILE *file;
    const char *filename = "datafile.txt";

    printf("Before setuid:\n");
    print_uids();

    file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
    }else {
        printf("File opened successfully\n");
    }

    if (setuid(getuid()) == -1) {
        perror("Error setting UID");
        exit(EXIT_FAILURE);
    }
    printf("After setuid:\n");
    print_uids();
    file = fopen(filename, "r");

    if (file == NULL) {
        perror("Error opening file after setuid");
    }else {
        printf("File opened successfully after setuid\n");
    }

    return 0;
}
