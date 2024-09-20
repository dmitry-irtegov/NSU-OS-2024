#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

void open(char* name){
    FILE* file = fopen(name, "r");
    if (file == NULL){
        perror("File didn`t open!\n");
        exit(-1);
    }
    else{
        printf("File %s opend!\n", name);
        fclose(file);
    }
}

int main(int argc, char **argv) {
    if (argc < 2){
        perror("ERROR: Not enough aruments!");
        exit(-1);
    }
    printf("Real UID: %d\nEffective UID: %d\n", getuid(), geteuid());
    open(argv[1]);
    if (setuid(geteuid())){
        perror("ERROR: can`t use setuid!");
    }
    open(argv[1]);
    return 0;
}