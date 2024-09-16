#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
extern char *tzname[];

void main()
{
    struct tm *sp;
    setenv("TZ", "America/Los_Angeles", 1);
    time_t now;
    (void) time( &now );
    printf("%s", ctime( &now ) );

    sp = localtime(&now);
    printf("%d:%02d %s %d.%d.%02d\n",
        sp->tm_hour, sp->tm_min,
        tzname[sp->tm_isdst], 
        sp->tm_mon + 1, sp->tm_mday, 
        sp->tm_year + 1900);
    exit(0);
}

