#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

extern char *tzname[];

int main()
{
    time_t now = 0;
    struct tm *sp = NULL;
    if (setenv("TZ", "America/Los_Angeles", 1) == -1)
    {
        perror("unable to set TZ variable");
        return 1;
    }
    now = time(NULL);
    printf("%s", ctime(&now));
    sp = localtime(&now);
    printf("%d/%d/%02d %d:%02d %s\n",
           sp->tm_mon + 1, sp->tm_mday,
           sp->tm_year + 1900, sp->tm_hour,
           sp->tm_min, tzname[sp->tm_isdst]);
    return 0;
}
