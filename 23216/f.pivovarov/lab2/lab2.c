#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

int main() {
    time_t now;

    /* As follows from man, timezone getting from
    TZ environment variable by tzset() function.

    This value used by ctime and localtime to
    convert timestamp into correct for current
    timezone time.
    */

    if (putenv("TZ=America/Los_Angeles") != 0) {
        perror("putenv");
        exit(EXIT_FAILURE);
    }

    if (!time(&now)) {
        perror("time");
        exit(EXIT_FAILURE);
    }

    char *date = ctime(&now);
    if (date == NULL) {
        perror("ctime");
        exit(EXIT_FAILURE);
    }

    printf("%s", date);

    exit(EXIT_SUCCESS);
}