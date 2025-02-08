#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <libgen.h>

int main() {
    srand(time(NULL));

    FILE* fd[2];
    if (p2open("sort -n", fd) == -1) {
        perror("p2open failed");
        exit(EXIT_FAILURE);
    }
    
    for (int _ = 0; _<100; _++) {
        fprintf(fd[0], "%d\n", rand()%100);
    }

    if (fclose(fd[0]) == EOF){
        perror("failed close fd[0]");
        exit(EXIT_FAILURE);
    }

    int count = 0;
    int num;
    for (int i = 0; i < 100; i++) {
        if (fscanf(fd[1], "%d\n", &num) == EOF) {
            perror("scanf failed");
            exit(EXIT_FAILURE);
        }
        printf("%d ", num);
        count++;
        if(count % 10 == 0) {
            putc('\n', stdout);
        }
    }

    if (fclose(fd[1]) == EOF){
        perror("failed close fd[1]");
    }

    exit(EXIT_SUCCESS);
}