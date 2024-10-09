#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
extern char *tzname[];

int main()
{
    time_t now;
    struct tm *sp;

    (void) time( &now );

    printf("Current local time: %s", ctime( &now ));

    setenv("TZ", "PST8PDT", 1);

    sp = localtime(&now);
    printf("In California (PDT): %d/%d/%d %d:%02d %s\n",
        sp->tm_mon + 1, sp->tm_mday,
        sp->tm_year + 1900, sp->tm_hour,
        sp->tm_min, tzname[sp->tm_isdst]);

    exit(0);
}