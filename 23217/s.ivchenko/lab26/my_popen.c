#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>


FILE* my_popen(char command[256], const char* mode) {
    int pipefd[2]; //pipefd[0] for read, pipefd[1] for write
    pid_t pid;

    if (strcmp(mode, "r") != 0 && strcmp(mode, "w") != 0) { //w - parent writing, r - parent reading
        perror("invalid argument");
        return NULL;
    }

    if (pipe(pipefd) == -1) {
        perror("pipe failed");
        return NULL;
    }

    if ((pid = fork()) == -1) {
        perror("fork failed");
        return NULL;
    }

    if (pid == 0) {
        if (strcmp(mode, "r") == 0) {
            close(pipefd[0]);
            dup2(pipefd[1], STDOUT_FILENO);
            close(pipefd[1]);
        } else {
            close(pipefd[1]);
            dup2(pipefd[0], STDIN_FILENO);
            close(pipefd[0]);
        }

        char *token;
        char *commands[100];
        int i = 0;

        token = strtok(command, " ");

        while (token != NULL) {
            commands[i] = token;
            i++;
            token = strtok(NULL, " ");
        }
        commands[i] = NULL;
        
        execvp(commands[0], commands);
        perror("execvp");
        exit(1);
    }

    FILE* pipe_ts;
    if (strcmp(mode, "r") == 0) {
        close(pipefd[1]);
        pipe_ts = fdopen(pipefd[0], "r");
    } else {
        close(pipefd[0]);
        pipe_ts = fdopen(pipefd[1], "w");
    }

    return pipe_ts;
}

int main(){
    char* string = "I'm just a simple English string";
    char command[256] = "tr '[:lower:]' '[:upper:]'";
    FILE* pipe;

    if ((pipe = my_popen(command, "w")) == NULL) {
        perror("popen failed");
        return 1;
    }

    fputs(string, pipe);
    pclose(pipe);
    return 0;
}