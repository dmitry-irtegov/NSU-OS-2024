#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
extern char *tzname[];

int main() {
    time_t now;

    // Проверка на ошибку при установке переменной среды
    if (putenv("TZ=America/Los_Angeles") != 0) {
        perror("putenv error");  // Должны быть двойные кавычки для строки
        exit(EXIT_FAILURE);
    }

    // Получение текущего времени и проверка на ошибку
    if (time(&now) == (time_t)(-1)) {  // Проверяем на -1, а не на 0
        perror("time error");
        exit(EXIT_FAILURE);
    }

    // Печать текущего времени в человекочитаемом формате
    printf("%s", ctime(&now));
    exit(EXIT_SUCCESS);
}

