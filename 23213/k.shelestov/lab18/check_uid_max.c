#include <stdio.h>
#include <unistd.h>
#include <limits.h>

int main() {
    long max_uid = sysconf(_SC_GETPW_R_SIZE_MAX);

    if (max_uid == -1) {
        perror("sysconf");
        return 1;
    }

    printf("MAX UID: %ld\n", max_uid);
    return 0;
}

