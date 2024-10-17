#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int main() {
    
    uid_t real_user_ID = getuid();
    uid_t effective_user_ID = geteuid();

    printf("Real UserID:\t%d\n", real_user_ID);
    printf("Effective UserID:\t%d\n", effective_user_ID);

    FILE *f = fopen("test.txt", "r");
    
    if (f == NULL) {
        perror("Cannot open file.");
        exit(1);
    } else {
        fclose(f);
    }

    if (seteuid(real_user_ID) == -1) {
        perror("Cannot set effective user ID");
        exit(1);
    }
    
    return 0;
}