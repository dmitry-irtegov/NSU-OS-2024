#include <stdio.h>
#include <time.h>
#include <stdlib.h>

int main() {
    time_t now;
    char * cTime=NULL;

    if (putenv("TZ=America/Los_Angeles") == -1) {
        perror("failed putenv");
        exit(EXIT_FAILURE);
    }

    if (time(&now) == -1) {
        perror("failed time");
        exit(EXIT_FAILURE);
    }

    cTime = ctime(&now);
    if (cTime == NULL){
        perror("failed ctime");
        exit(EXIT_FAILURE);
    }

    printf("%s", cTime);

    exit(EXIT_SUCCESS);
}
