#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <stdlib.h>

void print_uids() {

    printf("Real UID: %d\n", getuid());
    printf("Effective UID: %d\n", geteuid());
}

int main() {

    printf("Before setuid:\n");
    print_uids();

    FILE* file = fopen("myfile.txt", "r");
    if (file == NULL) {
        perror("Error opening file");
    }
    else {
        fclose(file);
    }

   
    if (setuid(getuid()) != 0) {
        perror("setuid failed");
        exit(1);
    }

   
    printf("After setuid:\n");
    print_uids();


    file = fopen("myfile.txt", "r");
    if (file == NULL) {
        perror("Error opening file after setuid");
    }
    else {
        fclose(file);
    }

    return 0;
}
