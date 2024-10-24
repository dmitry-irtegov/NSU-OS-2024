#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    // Проверка на наличие аргументов
    if (argc < 2) {
        perror("Too few arguments");
        return EXIT_FAILURE;
    }

    // Создаем новый процесс
    pid_t pid = fork();

    if (pid < 0) {
        // Ошибка при создании процесса
        perror("Fork error");
        return EXIT_FAILURE;
    }

    if (pid == 0) {
        // Мы в дочернем процессе
        // Заменяем текущий процесс на команду, переданную в аргументах
        if (execvp(argv[1], &argv[1]) == -1){
            perror("Execvp error");
            exit(EXIT_FAILURE);
        };
    } else {
        // Мы в родительском процессе
        int status;
        // Ожидаем завершения дочернего процесса
        waitpid(pid, &status, 0);

        // Проверяем, завершился ли процесс нормально
        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            printf("Child process exit code: %d\n", exit_code);
        } else if (WIFSIGNALED(status)) {
            int signal = WTERMSIG(status);
            printf("Child process terminated by signal: %d\n", signal);
        } else {
            printf("Error with child process\n");
        }
    }

    return EXIT_SUCCESS;
}