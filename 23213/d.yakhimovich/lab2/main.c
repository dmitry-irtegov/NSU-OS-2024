#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

extern char *tzname[];

int main()
{
    time_t now;
    struct tm *sp;

    setenv("TZ", "PST8PDT", 1);

    (void)time(&now);
    sp = localtime(&now);
    printf("%02d/%02d/%d %02d:%02d:%02d %s\n",
           sp->tm_mon + 1, sp->tm_mday,
           sp->tm_year + 1900, sp->tm_hour,
           sp->tm_min, sp->tm_sec, tzname[sp->tm_isdst]);
    return 0;
}
