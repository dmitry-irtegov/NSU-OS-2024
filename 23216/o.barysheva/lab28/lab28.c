#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>

int main() {
    srand((unsigned int)time(NULL));

    FILE* fp[2];

    if (p2open("sort -n", fp) == -1) {
        perror("Failed to open pipe");
        return EXIT_FAILURE;
    }

    for (int i = 0; i < 100; i++) {
        if (fprintf(fp[0], "%d\n", rand() % 100) < 0) {
            perror("Failed to write to pipe");
            fclose(fp[0]);
            fclose(fp[1]);
            return EXIT_FAILURE;
        }
    }

    if (fclose(fp[0]) == EOF) {
        perror("Failed to close input pipe");
        fclose(fp[1]); 
        return EXIT_FAILURE;
    }

    int count = 0, number;
    while (fscanf(fp[1], "%d", &number) == 1) {
        printf("%2d ", number);
        count++;
        if (count % 10 == 0) {
            putchar('\n');
        }
    }

    if (ferror(fp[1])) {
        perror("Error reading from pipe");
        fclose(fp[1]);
        return EXIT_FAILURE;
    }

    if (fclose(fp[1]) == EOF) {
        perror("Failed to close output pipe");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}