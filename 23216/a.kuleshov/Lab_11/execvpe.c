#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>

extern char **environ;

int execvpe(const char *file, char *const argv[], char *const envp[]) {
    // Сохраняем текущее значение environ
    char **old_environ = environ;

    // Заменяем его на новое значение envp
    environ = (char **)envp;

    // Вызываем execvp с новым окружением
    int result = execvp(file, argv);

    // Если execvp завершился с ошибкой, восстанавливаем старую среду
    environ = old_environ;

    // Возвращаем результат выполнения execvp (он не возвращает, если команда успешна)
    return result;
}

int main() {
    // Создаем массив аргументов для команды
    char *args[] = {"env", NULL}; // Команда "env" выводит все переменные окружения

    // Задаём новое окружение
    char *new_envp[] = {"PATH=/bin:/usr/bin", NULL};

    // Создаем дочерний процесс
    pid_t pid = fork();

    if (pid < 0) {
        // Ошибка создания процесса
        perror("Failed to fork");
        return 1;
    } else if (pid == 0) {
        // Дочерний процесс: проверяем работоспособность функции
        execvpe("env", args, new_envp);

        // Если execvpe не выполнится, выводим ошибку
        perror("Failed to execute execvpe");
        return 1;
    } else {
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
        } else {
            // Сообщение о завершении дочернего процесса с ошибкой
            printf("\nChild process exited with error\n");
        }
    }
    return 0;
}