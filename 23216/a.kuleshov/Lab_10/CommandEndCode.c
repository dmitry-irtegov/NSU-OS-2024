#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>

int main(int argc, char *argv[]) {
    // Проверяем, что передана хотя бы одна команда
    if (argc < 2) {
        printf("There have to be at least 1 argument\n");
        return 1;
    }

    // Создаем дочерний процесс
    pid_t pid = fork();

    switch (pid) {
        case -1:
            // Ошибка создания процесса
            perror("Failed to fork");
            return 1;

        case 0:
            // Дочерний процесс: выполняем команду argv[1] с аргументами &argv[1]
            execvp(argv[1], &argv[1]);

            // Если execvp не выполнится, выводим ошибку
            perror("Failed to execute execvp");
            return 1;

        default:
            // Родительский процесс: ожидаем завершения дочернего процесса
            int status;
            pid_t wait_result = wait(&status);

        if (wait_result == -1) {
            // Ошибка при ожидании дочернего процесса
            perror("Failed to wait for the child process");
            return 1;
        }

        // Проверяем, завершился ли дочерний процесс нормально
        if (WIFEXITED(status)) {
            // Печать после завершения дочернего процесса
            printf("\nChild process exited with code %d\n", WEXITSTATUS(status));
            return 0;
        }
        // Сообщение о завершении дочернего процесса с ошибкой
        printf("\nChild process exited with error\n");
    }

    return 0;
}