#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main() {
    setenv("TZ", "America/Los_Angeles", 1);
    tzset(); // Переменная окружения TZ была изменена, обновляем данные о времени
    time_t now;
    (void)time(&now); 
    //printf("current Califor time: %s", ctime(&now));
    struct tm *sp;
    sp = localtime(&now);
    printf("cur time Califor: %d/%d/%04d %02d:%02d\n",
           sp->tm_mon + 1, sp->tm_mday,
           sp->tm_year + 1900,  // Исправлено на правильный год
           sp->tm_hour,
           sp->tm_min);
    return 0;
}
