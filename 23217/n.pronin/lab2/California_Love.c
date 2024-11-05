#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

int main() {
    time_t now;        
    struct tm *sp;      
                       
    setenv("TZ", "PST8PDT", 1);   
    tzset();            
                        
    (void) time(&now);  

    printf("UTC time: %s", ctime(&now));  // Выводим время в формате UTC
                       
    sp = localtime(&now);  
    printf("Pacific Time: %d/%d/%02d %d:%02d %s\n",
           sp->tm_mon + 1, sp->tm_mday,
           sp->tm_year + 1900,  
           sp->tm_hour, sp->tm_min, sp->tm_isdst ? "PDT" : "PST");
 
    return 0;
}
