#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int main(){
    FILE *file = fopen("test.txt", "r");
    if(file == NULL){
        perror("Error: couldn't open file!");
    } else {
        fclose(file);
    }

    uid_t ruid = getuid();
    uid_t euid = geteuid();
    printf("Real user id: %u\n", ruid);
    printf("Effective user id: %u\n", euid);

    if(setuid(ruid) == -1) {
        perror("Error: couldn't set user id!");
    }

    file = fopen("test.txt", "r");
    if(file == NULL){
        perror("Error: couldn't open file!");
    } else {
        fclose(file);
    }

    ruid = getuid();
    euid = geteuid();
    printf("Real user id: %u\n", ruid);
    printf("Effective user id: %u\n", euid);
}