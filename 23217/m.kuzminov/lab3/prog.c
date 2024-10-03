#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

void openFile() {
    FILE* file = fopen("file", "r");
    if(file == NULL) {
        perror("Can't open file");
        
    } else {
        fclose(file);
    }
}

int main() {
    printf("real id: %d\neffective id: %d\n", getuid(), geteuid());

    openFile();

    if(setuid(getuid()) == -1) {
        perror("setuid failed");
        exit(1);
    }

    printf("real id: %d\neffective id: %d\n", getuid(), geteuid());

    openFile();

    return 0;
}

