#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

void printIDs() {
    uid_t uid = getuid();
    uid_t euid = geteuid();
    printf("UID: %d\n", uid);
    printf("EUID: %d\n", euid);
}

void tryToOpenFile(char* nameOfFile) {
    printf("Reading file %s\n", nameOfFile);
    FILE* fStream = fopen(nameOfFile, "r");
    if (fStream) {
        printf("The file %s was read successfully\n", nameOfFile);
        if (fclose(fStream) != 0) {
            perror("Failed to close a file.. ");
        }
        else {
            printf("A file %s was closed successfully\n", nameOfFile);
        }
    }
    else {
        perror("Failed to read file.. ");
    }
    printf("\n");
}

void changeEUID(uid_t newEUID) {
    printf("Setting EUID to %d\n", newEUID);
    if (setuid(newEUID) == 0) {
        printf("Setting EUID to %d was done successfully\n", newEUID);
    }
    else {
        perror("Failed to set EUID.. ");
    }
    printf("\n");
}

int main() {
    char nameOfFile[5] = "file";
    printIDs();
    tryToOpenFile(nameOfFile);
    changeEUID(getuid());
    printIDs();
    tryToOpenFile(nameOfFile);
    return 0;
}
