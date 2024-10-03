#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <errno.h>

int main() {
    
    uid_t real_user_ID = getuid();
    uid_t effective_user_ID = geteuid();

    printf("Real ID:\t%d\n", real_user_ID);
    printf("Effective ID:\t%d\n", effective_user_ID);

    FILE *file = fopen("file", "r");
    if (file == NULL) {
        perror("Can't open file");
    } else {
        printf("File opened!\n");
        if (fclose(file) == EOF) {
            perror("fclose fail");
            return 1;
        }
    }

    if (seteuid(real_user_ID) == -1) {
        perror("Error setting effective user ID");
        return 1;
    }

    effective_user_ID = geteuid();
    
    printf("\n");
    printf("Real ID:\t%d\n", real_user_ID);
    printf("Effective ID:\t%d\n", effective_user_ID);

    file = fopen("file", "r");
    if (file == NULL) {
        perror("Can't open file");
    } else {
        printf("File opened!\n");
        if (fclose(file) == EOF) {
            perror("fclose fail");
            return 1;
        }
    }

    return 0;
}
