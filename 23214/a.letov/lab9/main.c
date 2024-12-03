#include <sys/types.h>
#include <wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

void main(void) {
    pid_t child_id, pid;
    int status;
    printf("pid:%d ppid:%d\n", getpid(), getppid());
    if((child_id=fork())==0){
        execlp("cat", "cat", "test.txt", (char*)0);
        perror("execlp error");
        exit(1);
    }
    else if(child_id==-1){
        perror("fork error");
        exit(2);
    }
    if((pid = wait(&status))!=-1){
        printf("\nchild id:%d\n", pid);
    }
    else {
        perror("wait error");
        exit(3);
    }
    exit(0);
}