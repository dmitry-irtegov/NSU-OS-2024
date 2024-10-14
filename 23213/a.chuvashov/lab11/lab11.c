#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>

extern char** environ;

int execvpe(const char *file, char *const argv[], char *const envp[]) {
    char** save_env = environ;
    environ = (char**)envp;
    execvp(file, argv);
    environ = save_env;
    return -1;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "No attributes\n");
        exit(-1);
    }

char *new_environ[] = {"PATH=/home/students/23200/a.chuvashov/NSU-OS-2024/23213/a.chuvashov/lab11/:/usr/bin:/usr/sbin:/sbin:/usr/gnu/bin", NULL};

    execvpe(argv[1], &argv[1], new_environ);
    perror("execvpe didn`t work right");
    exit(1);
}
