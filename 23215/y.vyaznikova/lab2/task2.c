#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <errno.h>

extern char *tzname[];

int main()
{
    time_t now;
    struct tm *sp;

    if ((now = time(NULL)) == -1)
    {
        perror("time");
        exit(1);
    }

    setenv("TZ", "America/Los_Angeles", 1);
    tzset();

    sp = localtime(&now);

    printf("%s", ctime(&now));

    printf("%02d/%02d/%04d %02d:%02d %s\n",
           sp->tm_mon + 1, sp->tm_mday,
           sp->tm_year + 1900, sp->tm_hour,
           sp->tm_min, tzname[sp->tm_isdst]);

    exit(0);
}