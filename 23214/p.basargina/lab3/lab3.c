#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

void printIds(uid_t realUID, uid_t effUID, char* filename) {
    printf("Real UID: %d, Effective UID: %d\n", realUID, effUID);

    FILE *file;
    if ((file = fopen(filename, "r"))  == NULL) {
        perror("Failed to open file");
    } else {
        fclose(file);
    }
}

int main() {
    uid_t rUID = getuid();
    uid_t eUID = geteuid();
    
    printIds(rUID, eUID, "secret.txt");

    if (setuid(rUID) == -1) {
        perror("Setuid failed");
        exit(1);
    }

    eUID = geteuid();

    printIds(rUID, eUID, "secret.txt");

    exit(0);
}
