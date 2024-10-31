#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Too few arguments. Try again.\n");
        exit(EXIT_FAILURE);
    }
    
    pid_t pid = fork();
    if (pid == -1) {
        perror("Error in \"fork()\"");
        exit(EXIT_FAILURE);
    }
    else if (pid == 0) { 
        if (execvp(argv[1], &argv[1]) == -1) {
            perror("Error in execvp(): ");
            exit(EXIT_FAILURE);
        }
    } 
    else if (pid > 0) {
        int stat_loc;
        if (waitpid(pid, &stat_loc, 0) == -1) {
            perror("Error in \"waitpid():\"");
            exit(EXIT_FAILURE);
        }

        if (WIFEXITED(stat_loc)) {
            printf("code: %d\n", WEXITSTATUS(stat_loc));
        }

        else {
            perror("Error in \"Child Procees\"");
            exit(EXIT_FAILURE);
        }
    } 
    return 0;
}