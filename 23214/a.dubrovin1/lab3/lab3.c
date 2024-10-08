#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

int main(){
    uid_t rid, eid;

    rid = getuid();
    eid = geteuid();
    printf("real and effective uid: %d %d\n", rid, eid);

    FILE* file = fopen("myfile", "r");

    if (file == NULL){
        perror("Cannot open file, sorry");
    } else {
        fclose(file);
    }
    
    if (setuid(getuid()) != 0){
        perror("Cannot set UID");
    }

    eid = geteuid();
    printf("real and effective uid: %d %d\n", rid, eid);

    file = fopen("myfile", "r");

    if (file == NULL){
        perror("Cannot open file, sorry");
    } else {
        fclose(file);
    }

    return 0;
}
