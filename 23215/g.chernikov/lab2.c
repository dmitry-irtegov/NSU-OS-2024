#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
int main()
{
    time_t now;
    setenv("TZ","UTC+7",1);
    (void)time(&now);
    printf("%s", ctime(&now));
    exit(0);
}