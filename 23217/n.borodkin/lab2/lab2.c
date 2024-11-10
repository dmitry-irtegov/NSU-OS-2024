#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main() {
    time_t current_time;          
    struct tm local_time;

    setenv("TZ", "PST8PDT", 1);
    tzset(); 

    current_time = time(NULL);

    localtime_r(&current_time, &local_time);

    printf("Current time in California (PST): %02d:%02d:%02d %02d-%02d-%04dn", 
        local_time.tm_hour, 
        local_time.tm_min, 
        local_time.tm_sec, 
        local_time.tm_mday, 
        local_time.tm_mon + 1,  
        local_time.tm_year + 1900);  

    return 0;
}