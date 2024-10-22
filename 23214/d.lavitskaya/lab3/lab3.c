#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>


int main(){
    FILE *file;
    uid_t ruid = getuid();
    uid_t euid = geteuid();
    printf("Real uid: %d Effective uid: %d\n", ruid, euid);
    if ((file = fopen("roblox.txt", "r")) == NULL){
        perror("failed fopen");
    }
    else{
        fclose(file);
    }

    if(setuid(ruid) == -1) {
        perror("failed setuid");
    }
    euid = geteuid();
    printf("Real uid: %d Effective uid: %d\n", ruid, euid);
    if ((file = fopen("roblox.txt", "r")) == NULL){
        perror("failed fopen");
    }
    else{
        fclose(file);
    }

    exit(0);
}