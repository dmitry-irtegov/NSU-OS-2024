#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <stdlib.h>

int main() {
    // Печать реальных и эффективных идентификаторов пользователя
    uid_t real_uid = getuid();       // Реальный ID пользователя
    uid_t effective_uid = geteuid(); // Эффективный ID пользователя
    printf("Реальный UID: %d, Эффективный UID: %d\n", real_uid, effective_uid);

    FILE *file = fopen("myfile.txt", "r");
    if (file == NULL) {
        perror("Ошибка при открытии файла");
    } else {
        printf("Файл успешно открыт\n");
        fclose(file);
    }

    if (setuid(real_uid) == -1) {
        perror("Ошибка при вызове setuid");
        exit(EXIT_FAILURE);
    }

    real_uid = getuid();
    effective_uid = geteuid();
    printf("После setuid - Реальный UID: %d, Эффективный UID: %d\n", real_uid, effective_uid);

    file = fopen("myfile.txt", "r");
    if (file == NULL) {
        perror("Ошибка при открытии файла после setuid");
    } else {
        printf("Файл успешно открыт после setuid\n");
        fclose(file);
    }

    return 0;
}
