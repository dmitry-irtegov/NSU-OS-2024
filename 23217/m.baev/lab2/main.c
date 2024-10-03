#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
extern char *tzname[];

int main() {
    time_t now;
    struct tm *sp;

    if(setenv("TZ", "America/Los_Angeles", 1) == -1) {

        fprintf(stderr, "setenv error\n");
        return 1;
    }
    //tzset();

    now = time(NULL);
    if (now == -1) {
        fprintf(stderr, "Get time error\n");
        return 1;
    }

    char *time_now = ctime(&now);
    if (time_now == NULL) {
        fprintf(stderr, "ctime error\n");
        return 1;
    }
    printf("%s", time_now);

    sp = localtime(&now);
    if (sp == NULL) {

        fprintf(stderr, "Get time error\n");
        return 1;
    }
    printf("%d/%d/%02d %d:%02d %s\n",
        sp->tm_mon + 1, sp->tm_mday,
        sp->tm_year, sp->tm_hour,
        sp->tm_min, tzname[sp->tm_isdst]);

    return 0;
}