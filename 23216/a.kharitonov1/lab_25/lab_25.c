#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#define   buffer   512

int main(){
    int fd[2]; pid_t pid;
    char buf[buffer];
    ssize_t msglen;
    if (pipe(fd) == -1) {
        perror("problem in pipecreate");
        exit(EXIT_FAILURE);
    }
    pid=fork();
    switch(pid){
        case -1:
            perror("problem in fork");
            exit(EXIT_FAILURE);
        case 0:
            if (close(fd[0]) == -1) {
                perror("problem in pipeclose");
                if (close(fd[1]) == -1) {
                    perror("problem in pipeclose");
                    exit(EXIT_FAILURE);
                }
                exit(EXIT_FAILURE);
            }
            while((msglen = read(0,buf,buffer)) >= 0){
                if (msglen == 0){
                    break;
                }
                if (buf[msglen-1] == '\n'){
                    if(write(fd[1], buf, msglen)==-1){
                        perror("problem in write");
                        if (close(fd[1]) == -1) {
                            perror("problem in pipeclose");
                            exit(EXIT_FAILURE);
                        }
                        exit(EXIT_FAILURE);
                    }
                    break;
                }
                if(write(fd[1], buf, msglen)==-1){
                    perror("problem in write");
                    if (close(fd[1]) == -1) {
                        perror("problem in pipeclose");
                        exit(EXIT_FAILURE);
                    }
                    exit(EXIT_FAILURE);
                }
            }
            if(msglen <0){
                perror("problem in read from terminal");
                if (close(fd[1]) == -1) {
                    perror("problem in pipeclose");
                    exit(EXIT_FAILURE);
                }
                exit(EXIT_FAILURE);
            }
            if (close(fd[1]) == -1) {
                perror("problem in pipeclose");
                exit(EXIT_FAILURE);
            }
            exit(EXIT_SUCCESS);
        default:
            
            if (close(fd[1]) == -1) {
                perror("problem in pipeclose");
                if (close(fd[0]) == -1) {
                    perror("problem in pipeclose");
                    exit(EXIT_FAILURE);
                }
                exit(EXIT_FAILURE);
            }
            while((msglen = read(fd[0], buf, buffer))>0){
                for(ssize_t i=0;i<msglen;i++){
                    buf[i]=toupper(buf[i]);
                }
                if (write(1,buf,msglen) == -1){
                    perror("problem in write");
                    if (close(fd[0]) == -1) {
                        perror("problem in pipeclose");
                        exit(EXIT_FAILURE);
                    }
                    exit(EXIT_FAILURE);
                }
            }
            if(msglen == -1){
                perror("problem in read");
                if (close(fd[0]) == -1) {
                    perror("problem in pipeclose");
                    exit(EXIT_FAILURE);
                }
                exit(EXIT_FAILURE);
            }
            
            if (close(fd[0]) == -1) {
                perror("problem in pipeclose");
                exit(EXIT_FAILURE);
            }
            exit(EXIT_SUCCESS);
    }
}
