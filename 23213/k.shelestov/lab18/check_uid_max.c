#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>
int main() {
    // Открываем файл, чтобы получить файловый дескриптор
    int fd = open("/", O_RDONLY);  // Открываем корневой каталог

    if (fd == -1) {
        perror("open");
        return 1;
    }

    long max_uid = fpathconf(fd, UID_MAX);  // Используем fpathconf

    if (max_uid == -1) {
        perror("fpathconf");
        close(fd);  // Закрываем файловый дескриптор
        return 1;
    }

    printf("Max UID: %ld\n", max_uid);
    close(fd);  // Закрываем файловый дескриптор
    return 0;
}

