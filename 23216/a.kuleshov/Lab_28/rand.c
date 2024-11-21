#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

int main() {
    FILE *fp[2];
    // Открываем каналы для взаимодействия с sort
    if (p2open("sort -n", fp) == -1) {
        perror("p2open");
        return EXIT_FAILURE;
    }

    // Инициализация генератора случайных чисел
    srand(time(NULL));

    // Генерация случайных чисел и запись их в канал fp[0] (stdin sort)
    for (int i = 0; i < 100; i++) {
        fprintf(fp[0], "%d\n", rand() % 100);
    }

    // Закрытие канала ввода, завершение записи в sort
    fclose(fp[0]);

    // Чтение отсортированных чисел из канала fp[1] (stdout sort)
    int number, count = 0;
    while (fscanf(fp[1], "%d", &number) != EOF) {
        printf("%d ", number);
        count++;
        if (count % 10 == 0) {
            printf("\n");
        }
    }

    // Закрытие канала вывода
    fclose(fp[1]);

    return EXIT_SUCCESS;
}
