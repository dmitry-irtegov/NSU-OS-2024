#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <stdlib.h>

void print_user_ids() 
{
    printf("Real UID: %d\n", getuid());
    printf("Effective UID: %d\n", geteuid());
}

int main() 
{
    int errno;
    FILE* file;
    const char* filename = "file";

    printf("Before the change of identifiers:\n");
    print_user_ids();

    file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
    }

    else {
        printf("File successfully opened.\n");
        fclose(file);
    }

    if (setuid(getuid()) == -1) {
        perror("Error executing setuid");
        exit(EXIT_FAILURE);
    }

    printf("After changing identifiers:\n");
    print_user_ids();

    file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file”");
    }

    else {
        printf("File successfully opened for the second time.\n");
        fclose(file);
    }

    return 0;
}
