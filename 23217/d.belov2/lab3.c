#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("filename error\n");
        return 1;
    }

    printf("UID: %d, EUID: %d\n", getuid(), geteuid());
    
    FILE *file = fopen(argv[1], "r");
    if (file == NULL) 
    {
        perror("error");
    } else {
        printf("success\n");
        fclose(file);
    }

    if (setuid(getuid()) == -1) {
        perror("setuid error");
        exit(EXIT_FAILURE);
    }

    printf("UID: %d, EUID: %d\n", getuid(), geteuid());

    file = fopen(argv[1], "r");
    if (file == NULL) {
        perror("error");
    } else {
        printf("success\n");
        fclose(file);
    }

    return 0;
}
