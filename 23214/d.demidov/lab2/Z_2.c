#include <stdio.h>
#include <time.h>
#include <stdlib.h>
extern char* tzname[];

int main() {
    time_t now;
    struct tm* sp;
    (void)time(&now);
    setenv("TZ","PST8",1);
    printf("%s", ctime(&now));
    sp = localtime(&now);
    printf("%d/%d/%02d %d:%02d %s\n",
        sp->tm_mon + 1, sp->tm_mday,
        sp->tm_year + 1900, sp->tm_hour,
        sp->tm_min, tzname[sp->tm_isdst]);
    for (int i = 0; i < 5; ++i){
        printf("%d %s\n", i, tzname[i]);
    }
    return 0;
}
