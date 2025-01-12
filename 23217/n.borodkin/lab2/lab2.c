#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
extern char* tzname[];

int main()
{
    time_t now;
    struct tm* sp;

    // Получаем текущее время
    (void)time(&now);

    // Устанавливаем временную зону на Калифорнию (Los Angeles)
    setenv("TZ", "America/Los_Angeles", 1);

    // Преобразуем время в строку и выводим
    printf("Current time (ctime): %s", ctime(&now));

    // Преобразуем текущее время в локальное
    sp = localtime(&now);

    // Выводим подробное время в заданном формате
    printf("Formatted time: %d/%d/%04d %02d:%02d:%02d %s\n",
        sp->tm_mon + 1,   // Месяц
        sp->tm_mday,      // День
        sp->tm_year + 1900, // Год
        sp->tm_hour,      // Часы
        sp->tm_min,       // Минуты
        sp->tm_sec,       // Секунды
        tzname[sp->tm_isdst]); // Временная зона

    exit(0);
}
