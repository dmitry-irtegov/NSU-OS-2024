#include <stdio.h>
#include <unistd.h>

// Функция для печати реальных и эффективных UID, а также попытки открытия файла
void check_and_open_file(const char *filename) {
    uid_t real_uid = getuid();
    uid_t effective_uid = geteuid();

    // Печатаем реальные и эффективные идентификаторы пользователя
    printf("Real UID: %d, Effective UID: %d\n", real_uid, effective_uid);

    // Пытаемся открыть файл
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
    } else {
        printf("File opened successfully.\n");
        fclose(file);
    }
}

int main() {
    const char *filename = "data.txt";

    // Первый вызов функции проверки UID и открытия файла
    check_and_open_file(filename);

    // Меняем эффективный идентификатор на реальный
    if (setuid(getuid()) != 0) {
        perror("Error setting UID");
        return 1;
    }

    // Второй вызов функции после изменения UID
    check_and_open_file(filename);

    return 0;
}
