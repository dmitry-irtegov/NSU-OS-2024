#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#define   buffer   8

int main(){
    int fd[2]; pid_t pid;
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
            char msgout[40]="tESt LiNe fOr laB_25 hElLO woRld\n";
            if (puts(msgout) <0){
                perror("problem in puts");
                exit(EXIT_FAILURE);
            }
            if (close(pipefd[0]) == -1) {
                perror("problem in pipeclose");
                exit(EXIT_FAILURE);
            }
            if(write(fd[1], msgout, 40)==-1){
                perror("problem in write");
                exit(EXIT_FAILURE);
            }
            if (close(pipefd[1]) == -1) {
                perror("problem in pipeclose");
                exit(EXIT_FAILURE);
            }
            exit(EXIT_SUCCESS);
        default:
            char buf[buffer];
            ssize_t msglen;
            if (close(pipefd[1]) == -1) {
                perror("problem in pipeclose");
                exit(EXIT_FAILURE);
            }
            while((msglen = read(fd[0], buf, buffer))>0){
                for(ssize_t i=0;i<msglen;i++){
                    buf[i]=toupper(buf[i]);
                }
                if (write(1,buf,msglen) == -1){
                    perror("problem in write");
                    exit(EXIT_FAILURE);
                }
            }
            if(msglen == -1){
                perror("problem in read");
                exit(EXIT_FAILURE);
            }
            
            if (close(pipefd[0]) == -1) {
                perror("problem in pipeclose");
                exit(EXIT_FAILURE);
            }
            exit(EXIT_SUCCESS);
    }
}
