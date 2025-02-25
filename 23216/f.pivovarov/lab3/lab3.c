#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int fileOpenClose(char *filename);

int main(int argc, char **argv) {
    // 1: prints RUID and EUID
    printf("RUID: %d, EUID: %d\n", getuid(), geteuid());

    // 2: try to open file by name (and close)
    if (argc < 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        exit(EXIT_SUCCESS);
    }

    if (fileOpenClose(argv[1]) != 0) {
        exit(EXIT_FAILURE);
    }

    // 3: set effective user id (EUID) equal to real user id (RUID)
    if (setuid(getuid()) != 0) {
        perror("setuid");
        exit(EXIT_FAILURE);
    }

    // 4: prints RUID and EUID
    printf("RUID: %d, EUID: %d\n", getuid(), geteuid());

    // 5: try to open file by name (and close)
    if (fileOpenClose(argv[1]) != 0) {
      exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}

int fileOpenClose(char *filename) {
    // try to open file in r-mode
    FILE *file = fopen(filename, "r");

    if (file == NULL) {
        perror("Cannot open file");
        return 1;
    }
    // try to close
    if (fclose(file) != 0) {
        perror("Cannot close file");
        return 1;
    }

    return 0;
}