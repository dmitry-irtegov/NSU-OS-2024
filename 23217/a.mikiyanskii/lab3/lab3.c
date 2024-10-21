#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>

void print_uids() {
    // Печать реального и эффективного идентификаторов пользователя
    printf("Real UID: %d\n", getuid());
    printf("Effective UID: %d\n", geteuid());
}

int main() {
    // Шаг 1: Печать текущих идентификаторов
    printf("Before setuid:\n");
    print_uids();

    // Шаг 2: Попытка открыть файл
    FILE* file = fopen("myfile.txt", "r");
    if (file == NULL) {
        perror("Error opening file");
    }
    else {
        printf("File opened successfully!\n");
        fclose(file);
    }

    // Шаг 3: Сделать реальный и эффективный идентификаторы одинаковыми
    if (setuid(getuid()) != 0) {
        perror("setuid failed");
        exit(1);
    }

    // Шаг 4: Печать идентификаторов после setuid
    printf("After setuid:\n");
    print_uids();

    // Шаг 5: Попытка снова открыть файл
    file = fopen("myfile.txt", "r");
    if (file == NULL) {
        perror("Error opening file after setuid");
    }
    else {
        printf("File opened successfully after setuid!\n");
        fclose(file);
    }

    return 0;
}
