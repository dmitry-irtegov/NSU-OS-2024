#include <stdio.h>
#include <unistd.h>

void fileOpenClose(char *filename);

int main(int argc, char **argv) {
    // 1: prints RUID and EUID
    printf("RUID: %d, EUID: %d\n", getuid(), geteuid());

    // 2: try to open file by name (and close)
    if (argc < 2) {
        printf("Usage: ./lab3 <filename>\n");
        return 1;
    }
    char *filename = argv[1];
    fileOpenClose(filename);

    // 3: set effective user id (EUID) equal to real user id (RUID)
    if (setuid(getuid()) != 0) {
        perror("setuid");
    }

    // 4: prints RUID and EUID
    printf("RUID: %d, EUID: %d\n", getuid(), geteuid());

    // 5: try to open file by name (and close)
    fileOpenClose(filename);

    return 0;
}

void fileOpenClose(char *filename) {
    // try to open
    FILE *file = fopen(filename, "r");

    if (file == NULL) {
        perror("Cannot open file");
    } else {

        // try to close
        int status = fclose(file);

        if (status != 0) {
            perror("Cannot close file");
        }
    }
}