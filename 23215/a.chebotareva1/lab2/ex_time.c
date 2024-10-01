#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
extern char *tzname[];

int main()
{
    time_t now;
    struct tm *sp;

    // Установка временной зоны на PST (Pacific Standard Time)
    // set environment
    if(setenv("TZ", "PST8PDT", 1) != 0){
         perror("Error: Couldn't set the environment");
         exit(-1);
    }
    
    tzset(); // Обновляем информацию о временной зоне

    (void) time(&now);
    
    printf("%s", ctime(&now));
    sp = localtime(&now);
    printf("%d/%d/%02d %d:%02d %s\n",
        sp->tm_mon + 1, sp->tm_mday,
        sp->tm_year + 1900, sp->tm_hour,
        sp->tm_min, tzname[sp->tm_isdst]);
    exit(0);
}