#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#define   MSGSIZE   40

int main(){
    int fd[2]; pid_t pid;
    if (pipe(fd) == -1) {
        perror("problem in pipecreate");
        exit(EXIT_FAILURE);
    }
    if ((pid=fork()) > 0) {  /* parent */
        char msgin[MSGSIZE];
        ssize_t msglen;
        if (close(pipefd[1]) == -1) {
            perror("problem in pipeclose");
            exit(EXIT_FAILURE);
        }
        if((msglen = read(fd[0], msgin, MSGSIZE))==-1){
            perror("problem in read");
            exit(EXIT_FAILURE);
        }
        for(ssize_t i=0;i<msglen;i++){
            msgin[i]=toupper(msgin[i]);
        }
        if (puts(msgin) <0){
            perror("problem in puts");
            exit(EXIT_FAILURE);
        }
        if (close(pipefd[0]) == -1) {
            perror("problem in pipeclose");
            exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);
    }
    else if (pid == 0) {      /* child */
        char msgout[MSGSIZE]="tESt LiNe fOr laB_25 hElLO woRld\n";
        if (puts(msgout) <0){
            perror("problem in puts");
            exit(EXIT_FAILURE);
        }
        if (close(pipefd[0]) == -1) {
            perror("problem in pipeclose");
            exit(EXIT_FAILURE);
        }
        if(write(fd[1], msgout, MSGSIZE)==-1){
            perror("problem in write");
            exit(EXIT_FAILURE);
        }
        if (close(pipefd[1]) == -1) {
            perror("problem in pipeclose");
            exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);
    }
    else {          /* cannot fork */
        perror("problem in fork");
        exit(EXIT_FAILURE);
    }
}
