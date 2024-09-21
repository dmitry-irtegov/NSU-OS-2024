#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

int main(int argc, char** argv){
    if (argc > 2 || argc < 2){
        perror("Error: must be only 1 argument: ./lab10 <file>");
        exit(-1);
    }
    printf("Child procces starded!\n");
    fork();
    FILE* file = open(argv[1], "r");
    if (file == NULL){
        perror("Error: can`t open file. Check it in your derictory.");
        exit(-1);
    }
    cat();
}
