#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

int main(int argc, char *argv[])
{
    FILE *file;

    if (argc < 2){
        puts("missing filename");
        exit(1);
    }

    printf("uid = %d and euid = %d\n", getuid(), geteuid());
    file = fopen(argv[1], "r");

    if (file == NULL) {
        perror(argv[0]);
        exit(2);
    }
    else {
        printf("first open!\n");
        fclose(file);
    }

    setuid(getuid());
    printf("after setuid: uid = %d, euid = %d\n", getuid(), geteuid() );
    
    file = fopen(argv[1], "r");
    if (file == NULL) {
        perror(argv[0]);
        exit(3);
    }
    else {
        printf("second open!\n");
        fclose(file);
    }
}