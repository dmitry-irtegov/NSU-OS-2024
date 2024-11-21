#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char** argv){
    if (argc < 2){
        fprintf(stderr, "Not enough arguments\n");
        return 1;
    }

    pid_t pid = fork();
    int status;

    switch (pid){
        case (-1):
            perror("Fork finished with error");
            return 1;
        
        case (0):
            fprintf(stdout, "I'm a child process, hello!\n\n");
            execvp(argv[1], &argv[1]);
            perror("Execvp finished with error");
            return 1;
        
        default:
            if (waitpid(pid, &status, 0) == -1){
                perror("Waitpid finished with error");
                return 1;
            }

            else{
                fprintf(stdout, "\nI'm a parent process, hello!\n");

                if (WIFEXITED(status)){
                    fprintf(stdout, "Process finished with code %d\n", WEXITSTATUS(status));
                }
                else if (WIFSIGNALED(status)){
                    fprintf(stdout, "Process terminated by signal with number %d\n", WTERMSIG(status));
                }
                
                return 0;
            }
    }
}
