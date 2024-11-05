#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

int main() {
    uid_t real_uid = getuid();
    uid_t effective_uid = geteuid();

    printf("real UID: %d\n", real_uid);
    printf("effective UID: %d\n", effective_uid);

    FILE *file = fopen("datafile", "r");
    if (file == NULL) {
        perror("fopen error");
    } else {
        printf("fopen success.\n");
        fclose(file);
    }

    if (setuid(real_uid) == -1) {
        perror("setuid error");
        return EXIT_FAILURE;
    }

    real_uid = getuid();
    effective_uid = geteuid();
    printf("setuid success.\n");
    printf("real UID: %d\n", real_uid);
    printf("effective UID: %d\n", effective_uid);

    file = fopen("datafile", "r");
    if (file == NULL) {
        perror("fopen error");
    } else {
        printf("fopen success.\n");
        fclose(file);
    }

    return EXIT_SUCCESS;
}

