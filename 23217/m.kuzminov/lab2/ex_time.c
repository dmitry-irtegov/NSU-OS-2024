#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
extern char *tzname[];

int main()
{
    if (setenv("TZ", "America/Los_Angeles", 1) != 0) {
        printf("setenv didn't change");
        exit(1);
    }
    tzset();


    time_t now;  
    struct tm *sp;

    (void) time( &now );

    char* ctime_result = ctime(&now);
    if (ctime_result == NULL) {
        printf("ctime return null");
        exit(1);
    }
    printf("%s", ctime_result);

    sp = localtime(&now);
    if(sp == NULL){
        printf("localtime return null");
        exit(1);
    }

    printf("%d/%d/%02d %d:%02d %s\n",
        sp->tm_mon + 1, sp->tm_mday,
        sp->tm_year+1900, sp->tm_hour,
        sp->tm_min, tzname[sp->tm_isdst]);
    exit(0);
}
