#include <stdio.h>
#include <time.h>
#include <stdlib.h>

extern char *tzname[];

int main() {
    setenv("TZ", "PST8PDT", 1);
    tzset();

    time_t now;
    struct tm *sp;

    if (time(&now) == (time_t)(-1))
    {
        perror("time error");
    }

    sp = localtime(&now);

    if(ctime(&now) == NULL){
        perror("ctime error");
    }

    printf("%s", ctime(&now));

    exit(0);
}