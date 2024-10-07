#include <stdlib.h>
#include <libgen.h>
#include <stdio.h>
#include <unistd.h>

#define NUMBERS 100




int main() {

    int numbers[NUMBERS];

    srand((unsigned int)time(NULL));
    for (int i = 0; i < NUMBERS; i++) {
        numbers[i] = rand() % 100;
    }

    FILE* fd[2];
    if(p2open("sort -n", fd) == -1) {
        fprintf(stderr, "Couldn`t open pipe to sort\n");
        return 1;
    }
    
    for(int i = 0; i < NUMBERS; i++) {
        fprintf(fd[0], "%d\n", numbers[i]);
    }
    
    fclose(fd[0]);
    for(int i = 0; i < NUMBERS; i++) {
        fscanf(fd[1], "%d", &numbers[i]);
    }
    int count = 1;
    for(int i = 0; i < NUMBERS; i++) {
        printf("%d ", numbers[i]);
        if(count % 10 == 0) {
            putc('\n', stdout);
        }
        count++;
    }

    (void)p2close(fd);

    return 0;
}
