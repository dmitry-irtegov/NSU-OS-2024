#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>

int main() {
    srand(time(NULL));

    FILE* fp[2];

    if (p2open("sort -n", fp) == -1) {
        perror("Failed to open pipe");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < 100; i++) {
        if (fprintf(fp[0], "%d\n", rand() % 100) < 0) {
            perror("Failed to write to pipe");
            exit(EXIT_FAILURE);
        }
    }

    if (fclose(fp[0]) == EOF) {
        perror("Failed to close input pipe");
        exit(EXIT_FAILURE);
    }

    int number;

    for (int i = 1; i <= 100; i++) { 
        if (fscanf(fp[1], "%d", &number) == EOF) {
            perror("Failed to read from pipe");
            exit(EXIT_FAILURE);
        }
        
        printf("%2d ", number);
        if (i % 10 == 0) {
            putchar('\n');
        }
    }

    if (fclose(fp[1]) == EOF) {
        perror("Failed to close output pipe");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}