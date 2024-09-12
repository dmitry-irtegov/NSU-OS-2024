#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

extern char** environ;

int execvpe(const char *file, char *const argv[], char* envp[]){
    char** old_environment = environ;
    environ = envp;
    execvp(file, argv);
    environ = old_environment;
    return -1;
}

int main(int argc, char* argv[]){
    if (argc < 2){
        perror("arguments don't found");
        exit(EXIT_FAILURE);
    }

    char* env[] = {"TZ=America/Los_Angeles"};

    execvpe(argv[1], &argv[1], env);
    perror("execvpe failed");
    exit(EXIT_FAILURE);
}