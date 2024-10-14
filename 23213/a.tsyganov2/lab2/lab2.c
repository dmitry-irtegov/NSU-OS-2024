#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
extern char *tzname[];

int main()
{
    time_t now;
    struct tm *sp;

    putenv("TZ=America/Los_Angeles");

    (void) time( &now );
    sp = localtime(&now);
    printf("%02d/%02d/%d %d:%02d %s\n",
        sp->tm_mon + 1, sp->tm_mday,
        sp->tm_year + 1900, sp->tm_hour,
        sp->tm_min, tzname[sp->tm_isdst]);
    exit(0);
}
