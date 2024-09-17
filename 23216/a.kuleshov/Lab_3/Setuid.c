#include <stdio.h>
#include <unistd.h>

int main() {
    FILE *file;
    uid_t real_uid, effective_uid;

    // Печатаем реальные и эффективные идентификаторы пользователя
    real_uid = getuid();
    effective_uid = geteuid();
    printf("Real UID: %d, Effective UID: %d\n", real_uid, effective_uid);

    // Пытаемся открыть файл
    file = fopen("data.txt", "r");
    if (file == NULL) {
        perror("Error opening file");
    } else {
        printf("File opened successfully.\n");
        fclose(file);
    }

    // Меняем эффективный идентификатор на реальный
    if (setuid(real_uid) != 0) {
        perror("Error setting UID");
        return 1;
    }

    // Повторяем шаги после изменения идентификатора
    real_uid = getuid();
    effective_uid = geteuid();
    printf("After setuid - Real UID: %d, Effective UID: %d\n", real_uid, effective_uid);

    // Повторяем попытку открытия файла
    file = fopen("data.txt", "r");
    if (file == NULL) {
        perror("Error opening file");
    } else {
        printf("File opened successfully.\n");
        fclose(file);
    }

    return 0;
}
