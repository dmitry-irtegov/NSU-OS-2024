#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
uid_t printid(char* str){
    uid_t r_uid = getuid();
    uid_t e_uid = geteuid();

    printf("%s:\nReal UID: %d\n", str, r_uid);
    printf("Effective UID: %d\n", e_uid);
    
    FILE *file = fopen("file.txt", "r");
    if (file == NULL) {
        perror("error openning file");
        exit(EXIT_FAILURE);
    }
    else{
        fclose(file);
    }
    return e_uid;
}



int main(){
    //first
    uid_t e_uid = printid("before");

    //set the same
    if(setuid(e_uid) != 0){
        perror("error setting uid");
        exit(EXIT_FAILURE);
    }

    //second
    printid("after");



    exit(EXIT_SUCCESS);
}   