#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>
#include <string.h>

int main() {
    int pipe_fd[2];
    char buffer[100];
    
    // Создаем канал
    if (pipe(pipe_fd) == -1) {
        perror("pipe");
        exit(1);
    }

    // Создаем подпроцесс
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(1);
    }

    if (pid == 0) {
        // Дочерний процесс (переводит в верхний регистр)
        close(pipe_fd[1]); // Закрываем конец канала для записи

        // Чтение данных из канала и вывод в верхнем регистре
        while (read(pipe_fd[0], buffer, sizeof(buffer)) > 0) {
            for (int i = 0; buffer[i] != '\0'; i++) {
                buffer[i] = toupper((unsigned char) buffer[i]); // Преобразуем символ в верхний регистр
            }
            printf("Полученный текст в верхнем регистре: %s\n", buffer);
        }

        close(pipe_fd[0]);
        exit(0);
    } else {
        // Родительский процесс (пишет текст в канал)
        close(pipe_fd[0]); // Закрываем конец канала для чтения

        // Записываем текст в канал
        const char *text = "Hello, this is a mixed Case Text!";
        write(pipe_fd[1], text, strlen(text) + 1); // Пишем строку в канал

        close(pipe_fd[1]); // Закрываем канал после записи
        wait(NULL); // Ждем завершения дочернего процесса
        exit(0);
    }

    return 0;
}
