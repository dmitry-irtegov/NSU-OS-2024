#include <stdio.h>
#include <unistd.h>

int main() {
    int *x = (int*) 0x10001;

    // Если exec вернул управление, значит произошла ошибка
    *x = 5;
    perror("err");
    return 0;
}
