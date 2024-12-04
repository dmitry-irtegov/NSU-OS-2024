#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>

void main(void){
    int fd[2];
    pid_t pid;
    if(pipe(fd)==-1){
        perror("pipe error");
        exit(1);
    }
    if((pid=fork())>0){
        close(fd[0]);
        char message[31]="TestInGerereakwebrawshahaha";
        write(fd[1], message, 31);
        close(fd[1]);
    }
    else if(pid==0){
        close(fd[1]);
        char buffer[5];
        int size;
        while((size=read(fd[0], buffer, 5))!=0){
            for(int i = 0;i<size;i++){
                buffer[i]=toupper(buffer[i]);
            }
            printf("%s", buffer);
        }
    }
    else{
        perror("fork error");
        exit(2);
    }
    exit(0);

}