#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <unistd.h>
#include <ulimit.h>
#include <sys/resource.h>
#include <errno.h> 

void open_and_close(char *fileName, char *mode){
    FILE * file = fopen(fileName, mode);
    if(file){
        printf("lol\n");
        fclose(file);
    }   else{
        perror("error opening");
        exit(EXIT_FAILURE);
    }
}

int main (int argc, char * argv[]){

    if(argc != 2){
        printf("wrong number of parametrs");
        exit(EXIT_FAILURE);
    }

    printf("pid : %d\n", getuid());
    printf("euid: %d\n", geteuid());

    open_and_close(argv[1], "r");

    int uid_set = setuid(getuid());
    if(uid_set == -1){
        perror("setuid was unsucsesfull");
        exit(EXIT_FAILURE);
    }
    printf("After setuid\npid : %d\n", getuid());
    printf("euid: %d\n", geteuid());


    open_and_close(argv[1], "r");

    exit(EXIT_SUCCESS);
}