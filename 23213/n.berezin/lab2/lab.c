#include <stdio.h>
#include <time.h>
#include <stdlib.h>

extern char *tzname[];

void main() {
    setenv("TZ", "America/Los_Angeles", 1);

    time_t now;
    (void)time(&now);

    printf("%s", ctime(&now));

    struct tm *sp;
    sp = localtime(&now);
    printf("%d/%d/%02d %d:%02d %s\n",
           sp->tm_mon + 1, sp->tm_mday,
           sp->tm_year, sp->tm_hour,
           sp->tm_min, tzname[sp->tm_isdst]);
    exit(0);
}
