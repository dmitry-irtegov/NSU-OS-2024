#include <stdio.h>
#include <time.h>
#include <stdlib.h>

extern char *tzname[];

int main() {
    setenv("TZ", "America/Los_Angeles", 1);
    tzset();

    time_t now;
    struct tm *sp;
    
    if (time(&now) == (time_t)(-1)){
        perror("Current calendar time has not been encoded as time_t object");
        exit(1);
    }

    sp = localtime(&now);

    char* t = ctime(&now);
    if(t == NULL){
        perror("ctime error");
        exit(2);
    }

    printf("%s", t);

    exit(0);
}