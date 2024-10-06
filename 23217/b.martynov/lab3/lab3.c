#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

void printUserIDs() {
    printf("The real user ID: %d\n", getuid());
    printf("The effective user ID: %d\n", geteuid());
}

int main() {
        const char* nameFile = "file";

        printUserIDs();

        FILE* file = fopen(nameFile, "rw");
        if (file == NULL) {
            perror("Can't open file");
        } else {
            printf("Success open file.\n");
            (void)fclose(file);
        }

        uid_t userID = getuid();
        int setError = setuid(userID);
        if (setError != 0) {
            printf("setuid() failed");
            exit(EXIT_FAILURE);
        } 

        printUserIDs();

        file = fopen(nameFile, "rw");
        if (file == NULL) {
            perror("Can't open file");
        } else {
            printf("Success open file.\n");
            (void)fclose(file);
        }
}
