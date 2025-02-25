#include <stdio.h>
#include <time.h>
#include <stdlib.h>

int main() {
    if (putenv("TZ=America/Los_Angeles") != 0) {
        perror("Time change error");
        return 1;
    }

    time_t now;

    if (time(&now) == -1) {
        perror("Function time error");
        return 1;
    }
    char *timestr = ctime(&now);
    if (timestr == NULL) {
        perror("Function ctime error");
        return 1;
    }

    printf("%s", timestr);

    return 0;
}