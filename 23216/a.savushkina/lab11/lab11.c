#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <unistd.h>
#include <ulimit.h>
#include <sys/time.h>
#include <sys/resource.h>

extern char **environ;

int execvpe(char *const file, char *const argv[], char * envp)
{
    char **previous_env = environ;
    int add_env = putenv(envp);
    if (add_env){
        perror("error while adding new enviroment.");
        exit(EXIT_FAILURE);
    }
    int new_process = execvp(file, argv);
    if (new_process == -1)
    {
        perror("error in execvp");
        exit(EXIT_FAILURE);
    }
    environ = previous_env;
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("too few arguments\n");
        exit(EXIT_FAILURE);
    }
    char * env = {"TZ=Asia/Novosibirsk"};

    int new_process = execvpe(argv[1], &argv[1], env);
    if (new_process)
    {
        printf("Error in creating a new process\n");
        exit(EXIT_FAILURE);
    }
}