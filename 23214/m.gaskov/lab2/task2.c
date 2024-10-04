#include <stdio.h>
#include <time.h>
#include <stdlib.h>

int main() {
    putenv("TZ=PST8PDT");

    time_t now;
    (void) time(&now);
    struct tm *sp;
    sp = localtime(&now);

    printf("%d/%d/%02d %d:%02d %s\n",
        sp->tm_mon + 1, sp->tm_mday,
        sp->tm_year + 1900, sp->tm_hour,
        sp->tm_min, tzname[sp->tm_isdst]);
    return 0;
}
