#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/wait.h>

#define BUF_SIZE 1024

int main() {
    int fd[2];
    if(pipe(fd) == -1) {
        perror("pipe failed");
        exit(-1);
    }

    pid_t pid = fork();
    char buf[BUF_SIZE];
    ssize_t rc;
    switch (pid) {
    case -1:
        perror("fork failed");
        exit(-1);
    case 0:
        close(fd[0]);
        while((rc = read(STDIN_FILENO, buf, BUF_SIZE)) > 0) {
            if (write(fd[1], buf, rc) == -1) {
                perror("write failed");
                exit(-1);
            }
        }

        if(rc == -1) {
            perror("read failed");
            exit(-1);
        }
        exit(0);
    default:
        close(fd[1]);
        while((rc = read(fd[0], buf, BUF_SIZE)) > 0) {
            for (int i = 0; i < rc; i++) {
                putchar(toupper((unsigned char)buf[i]));
            }
        }

        if(rc == -1) {
            perror("read failed");
            exit(-1);
        }
        if (wait(NULL) == -1) {
            perror("wait failed");
            exit(-1);
        }
        exit(0);
    }    
}
