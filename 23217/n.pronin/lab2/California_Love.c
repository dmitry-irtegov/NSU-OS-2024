#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

int main() {
    time_t now;         // количество секунд, прошедших с 1 января 1970 года
    struct tm *sp;      // указатель на структуру типа tm, которая используется 
                        // для хранения локального времени в более удобном для нас виде.

                        // Устанавливаем временную зону на Pacific Time (PST/PDT)
    putenv("TZ=PST8PDT");  // Для Windows вместо setenv, устанавливает переменную окружения 
    tzset();            // Применить новое значение TZ
                        // обновляет часовой пояс на основе переменной окружения TZ, применяя её новое значение.
    (void) time(&now);  // возвращает текущее время в секундах с момента эпохи Unix

    printf("UTC time: %s", ctime(&now));  // Выводим время в формате UTC
                       // ctime(&now) — преобразует значение time_t в строку с форматированным представлением времени в формате UTC.
    sp = localtime(&now);  // Преобразуем в местное время (Pacific Time)
    printf("Pacific Time: %d/%d/%02d %d:%02d %s\n",
           sp->tm_mon + 1, sp->tm_mday,
           sp->tm_year + 1900,  // Год корректируется на +1900
           sp->tm_hour, sp->tm_min, sp->tm_isdst ? "PDT" : "PST");

   
    printf("Press Enter to exit...");
    getchar();  

    return 0;
}
