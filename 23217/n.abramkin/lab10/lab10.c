#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <command> [arguments...]\n", argv[0]);
        return 1;
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("Fork failed");
        return 1;
    } else if (pid == 0) {
        // Дочерний процесс: выполняем команду с аргументами
        execvp(argv[1], &argv[1]);
        
        perror("Exec failed");
        exit(1);
    } else {
        // Родительский процесс: ждём завершения дочернего процесса
        int status;
        if (waitpid(pid, &status, 0) == -1) {
            perror("Waitpid failed");
            return 1;
        }

        // Проверяем, как завершился дочерний процесс
        if (WIFEXITED(status)) {
            printf("Child process exited with code %d\n", WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("Child process terminated by signal %d\n", WTERMSIG(status));
        } else {
            printf("Child process did not exit normally\n");
        }
    }

    return 0;
}
