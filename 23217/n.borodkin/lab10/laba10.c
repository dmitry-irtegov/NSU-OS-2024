#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <command> [args...]\n", argv[0]);
        return 1;
    }

    pid_t pid = fork();  // Создаем дочерний процесс

    if (pid < 0) {
        perror("fork failed");
        return 1;
    }

    if (pid == 0) {
        
        execvp(argv[1], &argv[1]);

        
        perror("execvp failed");
        return 1;
    } else {
        
        int status;
        if (waitpid(pid, &status, 0) == -1) {
            perror("waitpid failed");
            return 1;
        }

        // Проверяем код завершения дочернего процесса
        if (WIFEXITED(status)) {
            printf("Child process exited with code %d\n", WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("Child process terminated by signal %d\n", WTERMSIG(status));
        } else {
            printf("Child process ended with an unknown status\n");
        }
    }

    return 0;
}