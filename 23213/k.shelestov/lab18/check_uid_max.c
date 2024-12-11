#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <sys/param.h>
#include <param.h>
int main() {
    long max_uid = sysconf(_SC_MAXUID);

    if (max_uid == -1) {
        perror("sysconf");
        return 1;
    }

    printf("MAX UID: %ld\n", max_uid);
    return 0;
}

