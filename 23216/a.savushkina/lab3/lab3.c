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
    }   
    else{
        perror("error opening");
    }
}

int main (){
    printf("pid : %d\n", getuid());
    printf("euid: %d\n", geteuid());

    open_and_close("test_file.txt", "r");

    setuid(getuid());
    printf("After setuid\npid : %d\n", getuid());
    printf("euid: %d\n", geteuid());


    open_and_close("test_file.txt", "r");

    exit(0);
}