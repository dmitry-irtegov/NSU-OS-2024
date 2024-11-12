#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
extern char** environ;

int execvpe(char* command, char* argv[], char* envp[]) {
    char** save_env = environ;
    environ = (char**) envp;
    printf("Executing %s...\n", command);
    execvp(command, argv);
    environ = save_env;
    return -1;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Not enough arguments for execution\n");
        exit(-1);
    }
    char *new_env[] = {"PATH=/home/students/23200/n.lebedev1/NSU-OS-2024/23214/n.lebedev1/lab11/:/usr/bin", NULL};
    char* command = argv[1];
    char** childArg = argv + 1;
    if (execvpe(command, childArg, new_env) == -1){
        printf("Failed to execute\n");
        exit(-1);
    }
}