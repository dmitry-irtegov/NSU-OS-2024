#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
extern char *tzname[];
extern time_t timezone;

int main()
{
    /*
    I can try to do it using gmtime and asctime, now - 8*60*60 
    but then there is the problem of how to display the correct time zone

    So i decided to change environment variable using setenv
    */
    setenv("TZ", "America/Los_Angeles", 1);//to change env var
    tzset();


    time_t now;
    struct tm *sp;

    (void) time( &now );


    printf("%s", ctime( &now ) );

    sp = localtime(&now);

    printf("%d/%d/%02d %d:%02d %s\n",
        sp->tm_mon + 1, sp->tm_mday,
        sp->tm_year, sp->tm_hour,
        sp->tm_min, tzname[sp->tm_isdst]);
    exit(0);
}