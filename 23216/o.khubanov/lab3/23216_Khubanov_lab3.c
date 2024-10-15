#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

// Функция, которая пытается открыть файл и выводит информацию об UID и EUID
int openFileWithInfo(char* filename, char* successMessage) {
    // Печать реального и эффективного идентификаторов пользователя
    printf("Real UID = %d, Effective UID = %d\n", getuid(), geteuid());

    // Попытка открыть файл для чтения
    FILE *filePointer = fopen(filename, "r");

    // Если не удалось открыть файл, выводим ошибку и возвращаем код ошибки
    if (filePointer == NULL) {
        perror(filename);  // Вывод ошибки с использованием имени файла
        return 1;  // Возврат 1, что указывает на ошибку
    }

    // Если файл успешно открыт, выводим сообщение об успехе
    puts(successMessage);

    // Закрываем файл
    fclose(filePointer);

    // Возвращаем 0, указывая на успешное выполнение
    return 0;
}

int main(int argc, char *argv[]) {
    // Проверяем, было ли передано имя файла как аргумент программы
    if (argc < 2) {
        puts("Filename is missing!");  // Сообщение об ошибке, если аргументов меньше двух
        exit(1);  // Выход с кодом 1
    }

    // Первая попытка открыть файл с выводом UID/EUID
    if (openFileWithInfo(argv[1], "File successfully opened in the first attempt!")) {
        exit(2);  // Если файл не удалось открыть, завершить с кодом 2
    }

    // Устанавливаем эффективный UID равным реальному UID
    if (setuid(getuid())) {
        perror("Error: Unable to reset effective UID to real UID");  // Сообщение об ошибке
        exit(3);  // Завершаем программу с кодом 3, если не удалось изменить UID
    }

    // Вторая попытка открыть файл после изменения UID
    if (openFileWithInfo(argv[1], "File successfully opened in the second attempt!")) {
        exit(2);  // Если файл не удалось открыть, завершить с кодом 2
    }

    exit(0);  // Успешное завершение программы
}

