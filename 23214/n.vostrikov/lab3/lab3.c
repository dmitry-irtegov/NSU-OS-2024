#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

int main() {
    printf("Real UID: %d, Effective UID: %d\n", getuid(), geteuid());
    FILE *file = fopen("data.txt", "r");
    if (file == NULL) {
        perror("Error opening file");
    } else {
        printf("File opened successfully\n");
        fclose(file);
    }
    setuid(getuid());
    printf("After setuid - Real UID: %d, Effective UID: %d\n", getuid(), geteuid());
    file = fopen("data.txt", "r");
    if (file == NULL) {
        perror("Error opening file after setuid");
    } else {
        printf("File opened successfully after setuid\n");
        fclose(file);
    }
    return 0;
}
