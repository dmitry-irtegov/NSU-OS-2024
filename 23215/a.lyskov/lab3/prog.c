#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>

int main() {
    printf("Real UID: %d\n", getuid());
    printf("Effective UID: %d\n", geteuid());

    FILE *file = fopen("file", "r+");
    if (file == NULL) {
        perror("Error opening file");
    } else {
        printf("File opened successfully.\n");
        fclose(file);
    }

    if (setuid(getuid()) == -1) {
        perror("Error setting UID");
        exit(EXIT_FAILURE);
    }

    printf("Real UID and Effective UID set to the same value.\n");
    printf("Real UID: %d\n", getuid());
    printf("Effective UID: %d\n", geteuid());

    file = fopen("file", "r+");
    if (file == NULL) {
        perror("Error opening file after UID change");
    } else {
        printf("File opened successfully after UID change.\n");
        fclose(file);
    }

    return 0;
}