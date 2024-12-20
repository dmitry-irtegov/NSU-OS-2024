#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

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
        fprintf(stderr, "not enough args: %s\n", argv[0]);
        exit(-1);
    }
    char* value = getenv("PATH");
    char* path = (char*) malloc((76 + strlen(value)) * sizeof(char));
    strcpy(path, "PATH=/home/students/23200/v.abramenko/NSU-OS-2024/23213/v.abramenko/lab11/:");
    strcat(path, value);
    char *new_env[] = {path, NULL};
    execvpe(argv[1], &argv[1], new_env);
    perror("execvpe failed");
    exit(-1);
}
