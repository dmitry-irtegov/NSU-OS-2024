#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    // Создаем новый процесс
    pid_t pid = fork();

    if (pid < 0) {
        // Ошибка при создании процесса
        perror("Fork error");
        return EXIT_FAILURE;
    }

    if (pid == 0) {
        // Мы в дочернем процессе
        // Выполняем cat
        if (execlp("cat", "cat", "text.txt", NULL) == -1) {
            perror("Execlp error");
            exit(EXIT_FAILURE);
        }
    } else {
        // Мы в родительском процессе
        int status;
        // Ожидаем завершения дочернего процесса
        if (waitpid(pid, &status, 0) == -1) {
            perror("Waitpid error");
            exit(EXIT_FAILURE);
        }

        // Проверяем, завершился ли процесс нормально
        if (WIFEXITED(status)) {
            printf("\nSuccess!\n");
        } else {
            printf("Error with child process\n");
        }
    }

    return EXIT_SUCCESS;
}