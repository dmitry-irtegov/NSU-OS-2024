#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <wait.h>

int main() {
    int fd[2], status, size;
    pid_t child;

    if (pipe(fd) == -1) {
        perror("pipe error");
        exit(1);
    }

    child = fork();
    
    if (child == -1) {
        perror("fork error");
        exit(2);
    }
    else if (child == 0) {
        char text[BUFSIZ];
        if((close(fd[1])) == -1){
            perror("Close error");
        }

        while ((size = read(fd[0], text, BUFSIZ)) > 0) {
            for (int i = 0; i < size; i++) {
                text[i] = toupper(text[i]);
            }
            printf("%s\n", text);
        }
        if (size == -1) {
            perror("read error");
        }
        if((close(fd[0])) == -1){
            perror("Close error");
        }
    } else {
        char msg[BUFSIZ] = "HelLo WorLd aNd evERyone!";
        if((close(fd[0])) == -1){
            perror("Close error");
        }        if (write(fd[1], msg, strlen(msg) + 1) == -1) {
            perror("write error");
        }

        if((close(fd[1])) == -1){
            perror("Close error");
        }

        if (wait(&status) == -1) {
            perror("wait error");
        }
    }

    exit(0);
}

