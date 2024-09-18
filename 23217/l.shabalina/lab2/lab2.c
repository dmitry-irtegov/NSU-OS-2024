#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

extern char *tzname[];

int main() {

    if (setenv("TZ", "America/Los_Angeles", 1) == -1) {
        perror("Set TZ failure");
        return 1;
    }

    time_t now;
    struct tm *sp;
    now = time(NULL);

    char *time_str = ctime(&now);
    if (time_str == NULL) {
        perror("ctime failure");
        return 1;
    }
    
    printf("%s", time_str);

    sp = localtime(&now);
     if (sp == NULL) {
        perror("localtime failure");
        return 1;
    }

    printf("%d/%d/%02d %d:%02d %s\n",
        sp->tm_mon + 1, sp->tm_mday,
        sp->tm_year % 100, sp->tm_hour,
        sp->tm_min, tzname[sp->tm_isdst]);

    exit(0);
}
 
