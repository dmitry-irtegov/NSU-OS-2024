#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main() {
    time_t current_time;          // Текущее время в секундах
    struct tm local_time;

    // Ставим переменную окружения на PST с учетом перехода на летнее время
    setenv("TZ", "PST8PDT", 1);
    tzset();  // Обновляем инфу о часовом поясе

    // Получаем текущее время, начиная с 1970 года 1 января
    current_time = time(NULL);
    
    // Преобразуем в локальное время с учетом установленного часового пояса
    localtime_r(&current_time, &local_time);

    printf("Current time in California (PST): %02d:%02d:%02d %02d-%02d-%04dn", 
        local_time.tm_hour, 
        local_time.tm_min, 
        local_time.tm_sec, 
        local_time.tm_mday, 
        local_time.tm_mon + 1,  // tm_mon начинается с 0, поэтому добавляем 1
        local_time.tm_year + 1900);  // tm_year хранит количество лет с 1900 поэтому добавил 1900

    return 0;
}
