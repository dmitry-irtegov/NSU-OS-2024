#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>


void test_file(const char* path) {
    FILE* file = fopen(path, "r");
    if (file == NULL) {
        perror("can`t open file");
    } else {
        printf("opened succesfuly\n");
        fclose(file);
        file = NULL;
    }
}


int main(int argc, char** argv) {

    if (argc < 2){
        fprintf(stderr, "not enough arguments\n");
        exit(-1);
    }

    uid_t uid = getuid();
    uid_t e_uid = geteuid();
    printf("uid: %d\n", uid);
    printf("euid: %d\n", e_uid);

    test_file(argv[1]);

    if(setuid(uid) == -1) {
        perror("setuid failed");
        exit(-1);
    }
    
    e_uid = geteuid();
    printf("uid: %d\n", uid);
    printf("euid: %d\n", e_uid);

    test_file(argv[1]);

    exit(0);
}
