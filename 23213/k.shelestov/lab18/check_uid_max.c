#include <stdio.h>
#include <unistd.h>
#include <limits.h>

int main() {
    long max_uid = sysconf(_SC_UID_MAX);

    if (max_uid == -1) {
        perror("sysconf");
        return 1;
    }

    printf("Максимальное значение UID: %ld\n", max_uid);
    return 0;
}

