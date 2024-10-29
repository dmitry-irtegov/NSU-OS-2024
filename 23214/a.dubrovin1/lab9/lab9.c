#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char** argv){
    if (argc != 2){
        fprintf(stderr, "Inappropriate number of arguments\n");
        return 0;
    }

    pid_t pid = fork();

    switch (pid){
        case (-1):
            perror("Fork finished with error");
            return 0;
        
        case (0):
            fprintf(stdout, "I'm a child process, hello!\n\n");
            execlp("cat", "cat", argv[1], NULL);
            perror("Execlp finished with error");
            return 0;
        
        default:
            if (wait(NULL) == -1){
                perror("Wait finished with error");
                return 0;
            }

            else{
                fprintf(stdout, "\nI'm a parent process, hello!\n");
                return 0;
            }
    }
}
