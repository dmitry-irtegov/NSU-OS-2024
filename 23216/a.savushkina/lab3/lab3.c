#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <unistd.h>
#include <ulimit.h>
#include <sys/resource.h>
#include <errno.h> 

void open_and_close(char *fileName, char *mode){
    extern errno;  
    FILE * file = fopen(fileName, mode);
    if(file){
        printf("lol\n");
        fclose(file);
    }   
    else{
        perror("error opening\n");
        fflush(errno);
    }
}

int main (){
    printf("pid : %d\n", getuid());
    printf("euid: %d\n", geteuid());

    open_and_close("test_file.txt", "r");

    int ret = setuid(geteuid());
    printf("After setuid\npid : %d\n", getuid());
    printf("euid: %d\n", geteuid());


    open_and_close("test_file.txt", "r");

    exit(0);
}