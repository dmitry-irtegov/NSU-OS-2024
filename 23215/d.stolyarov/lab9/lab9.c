#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <wait.h>
int main(int argc, char* argv[])
{
    printf("%d - %d\n", getpid(), getppid());
    if(argc < 2){
        perror("Missing filename");
        exit(1);
    }
    if(strlen(argv[1]) > 95){
        perror("Too long filename");
        exit(2);
    }
    pid_t cid, ret = 123;
    int status = 234;
    char cmd[100] = "cat ";
    if((cid = fork()) == 0){
        //for(int i = 0; i < 100000; i++);
        printf("child's part:\n");
        strcat(cmd, argv[1]);
        execlp("sh", "sh", "-c", cmd, (char *) 0);
        perror("Sh error");
        exit(3);
    }
    else if(cid > 0){
        if(argc >= 3){
            printf("parent waiting\n");
            ret = wait(&status);
        }
        printf("Parent's text: %d - %d - %d\n", cid, ret, status);
        exit(0);
    }
    else{
        perror("Fork error");
        exit(4);
    }
}