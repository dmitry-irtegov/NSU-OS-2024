#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
extern char *tzname[];

int main()
{
    time_t now;
    struct tm *sp, *pst;
    putenv("TZ=America/Los_Angeles");
    tzset();
    (void) time( &now );

    fprintf(stderr, "ctime: %s\n", ctime( &now ) );
    return 0;
}
