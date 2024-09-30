#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
    uid_t real_uid = geteuid();
    uid_t effective_uid = getuid();

    printf("Real user ID: %u\n", real_uid);
    printf("Effective user ID: %u\n", effective_uid);

    if (fopen("file", "r") == NULL) {
        perror("Failed to open file");
        return 1;
    }

    fclose(fopen("file", "r"));
    printf("File opened successfully.\n");

    // Чтобы совпадение реального и эффективного ID пользователя
    setuid(real_uid);

    real_uid = geteuid();
    effective_uid = getuid();

    printf("Real user ID: %u\n", real_uid);
    printf("Effective user ID: %u\n", effective_uid);

    return 0;
}
