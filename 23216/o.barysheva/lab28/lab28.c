#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <libgen.h>

int main() {
    // Инициализация генератора случайных чисел
    srand(time(NULL));

    // Открываем каналы для взаимодействия с sort
    FILE* fd[2];
    if (p2open("sort -n", fd) == -1) {
        perror("p2open failed");
        exit(EXIT_FAILURE);
    }

    // Генерация и запись случайных чисел
    for (int i = 0; i < 100; i++) {
        if (fprintf(fd[0], "%d\n", rand() % 100) < 0) {
            perror("Failed to write to pipe");
            fclose(fd[0]);
            fclose(fd[1]);
            exit(EXIT_FAILURE);
        }
    }

    // Закрытие канала записи
    if (fclose(fd[0]) == EOF) {
        perror("Failed to close fd[0]");
        fclose(fd[1]); // Попытка закрыть выходной поток перед завершением
        exit(EXIT_FAILURE);
    }

    // Чтение и вывод отсортированных чисел
    int number, count = 0;
    while (fscanf(fd[1], "%d", &number) == 1) {
        printf("%d ", number);
        count++;
        if (count % 10 == 0) {
            putchar('\n');
        }
    }

    if (ferror(fd[1])) {
        perror("Error reading from pipe");
        fclose(fd[1]);
        exit(EXIT_FAILURE);
    }

    // Закрытие канала чтения
    if (fclose(fd[1]) == EOF) {
        perror("Failed to close fd[1]");
        exit(EXIT_FAILURE);
    }

    // Завершение программы
    return EXIT_SUCCESS;
}