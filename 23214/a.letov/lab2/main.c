#include <stdio.h>
#include <time.h>
#include <stdlib.h>
extern char *tzname[];
 
int main(void)
{
    time_t now;
    struct tm* sp;
    (void) time(&now);
    setenv("TZ", "PST8", 1);
    sp = localtime(&now);
    printf("%d/%d/%02d %d:%02d %s\n",
    sp->tm_mon + 1, sp->tm_mday,
    sp->tm_year+1900, sp->tm_hour,
    sp->tm_min, tzname[sp->tm_isdst]);
    return 0;
}