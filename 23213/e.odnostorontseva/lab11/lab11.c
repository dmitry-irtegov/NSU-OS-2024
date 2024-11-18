#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

extern char **environ;

int execvpe(const char* filename, char** argv, char* const envp[]) {
    char** tmp_environ = environ;
    environ = (char**)envp;
    execvp(filename, argv);
    environ = tmp_environ;
    return -1;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "No attribute file\n");
        exit(1);
    }

    char *new_env[] = {
        "PATH=/home/students/23200/e.odnostorontseva/NSU-OS-2024/23213/e.odnostorontseva/lab11/:/usr/bin:/usr/sbin:/sbin:/usr/gnu/bin", 
        NULL};

    execvpe(argv[1], &argv[1], new_env);
    perror("Execvpe failed");
    exit(-1);
}

