#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

void openFile() {
    FILE* file = fopen("datafile", "r");
    if (file == NULL) {
        perror("Unable to open file");
    }
    else {
        fclose(file);
    }
}

int main() {
    printf("Real ID: %d\nEffective ID: %d\n", getuid(), geteuid());

    openFile();

    if (setuid(getuid()) == -1) {
        perror("Setuid failed");
        exit(1);
    }

    printf("Real ID: %d\nEffective ID: %d\n", getuid(), geteuid());

    openFile();

    return 0;
}
