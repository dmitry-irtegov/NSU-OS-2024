#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <libgen.h>

int main() {
    srand((unsigned int)time(NULL));

    FILE* fp[2];
    if (p2open("sort -n", fp) == -1) {
        printf("p2open() failed.\n");
        exit(-1);
    }

    for (int i = 0; i < 100; i++) {
        fprintf(fp[0], "%d\n", rand() % 100);
    }

    fclose(fp[0]);

    int number;
    for (int i = 0; i < 100; i++) {
        fscanf(fp[1], "%d", &number);
        printf("%2d", number);

        if (i % 10 == 9) {
            printf("\n");
        }

        else {
            printf(" ");
        }
    }

    exit(0);
}
