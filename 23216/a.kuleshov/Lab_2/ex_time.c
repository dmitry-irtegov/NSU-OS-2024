#include <stdio.h>
#include <time.h>
#include <stdlib.h>
extern char *tzname[];

main()
{
    if (putenv("TZ=PST8PDT")==0) {
         printf("Time change success\n");
    }

    time_t now;
    struct tm *sp;

    (void) time( &now );

    printf("%s", ctime( &now ) );

    sp = localtime(&now);
    printf("%02d/%02d/%02d %d:%02d %s\n",
        sp->tm_mon + 1, sp->tm_mday,
        sp->tm_year + 1900, sp->tm_hour,
        sp->tm_min, tzname[sp->tm_isdst]);
    exit(0);
}
