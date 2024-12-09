#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
int main(int argc, char *argv[]){
    int t = 1;
    if(argc > 1){

        for(int i = 0; argv[1][i] != 0; i++){
            t += argv[1][i];
        }
        
    }
    srand(t);
    FILE *fptrs[2];
    if(p2open("sort -n", fptrs) == -1){
        perror("p2open error");
        exit(1);
    }
    for(int i = 0; i < 100; i++){
        t = rand()%100;
        if(fprintf(fptrs[0], "%d\n", t) == EOF){
            perror("Pipe printing error");
            exit(2);
        }
    }
    if(fclose(fptrs[0]) == EOF){
        perror("Pipe closing error");
        exit(3);
    }
    int j = 0;
    while(fscanf(fptrs[1], "%d", &t) != EOF){
        printf("%d ", t);
        j++;
        if(j == 10){
            j = 0;
            printf("\n");
        }
    }
    exit(0);
}