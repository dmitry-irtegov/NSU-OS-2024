#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int main(){
    uid_t ruid = getuid();
    uid_t euid = geteuid();
    printf("Real user id: %u\n", ruid);
    printf("Effective user id: %u\n", euid);

    FILE *file = fopen("testfile", "r");
    if(file == NULL){
        perror("Couldn't open file");
    } else {
        printf("File has opened successfully\n");
        fclose(file);
    }

    if(setuid(ruid) == -1) {
        perror("Couldn't set user id");
    }

    ruid = getuid();
    euid = geteuid();
    printf("Real user id: %u\n", ruid);
    printf("Effective user id: %u\n", euid);
    
    file = fopen("testfile", "r");
    if(file == NULL){
        perror("Couldn't open file");
    } else {
        printf("File has opened successfully\n");
        fclose(file);
    }
}