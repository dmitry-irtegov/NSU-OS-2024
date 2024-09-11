#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

extern char *tzname[];

void main() {
    time_t now;
    struct tm *sp;

    /* As follows from man, timezone getting from
    TZ environment variable by tzset() function.

    This value used by ctime and localtime to
    convert timestamp into correct for current
    timezone time.
    */
    if (putenv("TZ=America/Los_Angeles") != 0) {
        exit(1);
    }

    (void)time(&now);

    printf("%s", ctime(&now));

    sp = localtime(&now);
    printf(
        "%d/%d/%02d %d:%02d %s\n",
        sp->tm_mon + 1,
        sp->tm_mday,
        sp->tm_year,
        sp->tm_hour,
        sp->tm_min,
        tzname[sp->tm_isdst]
    );
    exit(0);
}