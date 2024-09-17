#include <stdio.h>
#include <time.h>
#include <stdlib.h>

main() {
    if (putenv("TZ=America/Los_Angeles") != 0) {
        perror("Time change error");
        exit(1);
    }

    time_t now;

    if (time(&now) == -1) {
        perror("Function time error");
        exit(1);
    }

    if (ctime(&now) == NULL) {
        perror("Function ctime error");
        exit(1);
    }

    printf("%s", ctime(&now));

    exit(0);
}