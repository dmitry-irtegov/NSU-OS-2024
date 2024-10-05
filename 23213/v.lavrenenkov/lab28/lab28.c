#include <stdlib.h>
#include <libgen.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#define NUMBERS 100

int main() {
    srand((unsigned int)time(NULL));
    FILE* fp[2];
    if(p2open("sort -n", fp) == -1) {
        fprintf(stderr, "Couldn`t open pipe to sort\n");
        return 1;
    }
    for(int i = 0; i < NUMBERS; i++) {
        fprintf(fp[0], "%d\n", rand() % 100);
    }
    fclose(fp[0]);

    int count = 1;
    for(int i = 0; i < NUMBERS; i++) {
        int now;
        fscanf(fp[1], "%d", &now);
        printf("%2d", now);
        if(count % 10 == 0) {
            printf("\n");
        } else {
            printf(" ");
        }
        count++;
    }

    return 0;
}
