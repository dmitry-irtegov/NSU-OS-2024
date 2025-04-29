#include <stdio.h>
#include <unistd.h>

extern char** environ;

int execvpe(const char* file, char *const argv[], char *const envp[]){
    char** tmp = environ;
    environ = (char**) envp;
    execvp(file, argv);
    environ = (char**) tmp;
    return -1;
}
