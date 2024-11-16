#include <stdio.h>
#include <time.h>

int main() {
    time_t current_time;         
    struct tm *utc_time;          
    struct tm pst_time;          

    current_time = time(NULL);      // Текущее время в сек от 1 января 1970
    
    utc_time = gmtime(&current_time);     //Преобразуем в UTC
    
    pst_time = *utc_time;    //Копируем
    
    pst_time.tm_hour -= 8;      // Переводим в PST (UTC-8)
    
//Если время стало отрицательным, то
    if (pst_time.tm_hour < 0) {
        pst_time.tm_hour += 24;   
        pst_time.tm_mday -= 1;   
    }

    //Выводим время
    printf("Current time in California (PST): %02d:%02d:%02d %02d-%02d-%04d\n", 
        pst_time.tm_hour, 
        pst_time.tm_min, 
        pst_time.tm_sec, 
        pst_time.tm_mday, 
        pst_time.tm_mon + 1,  // tm_mon начинается с 0, поэтому добавляем 1
        pst_time.tm_year + 1900);  // в tm_year лежит число лет прошедших с 1900 , поэтому добавляем 1900

    return 0;
}