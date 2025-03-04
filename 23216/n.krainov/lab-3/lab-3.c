#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

int tryToOpen(FILE* file, char* filename, char* message){
    printf("uid = %d and euid = %d\n", getuid(), geteuid());
    file = fopen(filename, "r");

    if (file == NULL) {
        perror(filename);
        return 1;
    }

    puts(message);
    fclose(file);
    return 0;

}

int main(int argc, char *argv[])
{
    FILE *file;

    if (argc < 2){
        puts("missing filename");
        exit(1);
    }

    if (tryToOpen(file, argv[1], "first open!")){
        exit(2);
    }

    if (setuid(getuid())){
        perror("failed to use setuid");
    }
    
    if (tryToOpen(file, argv[1], "second open!")){
        exit(2);
    }
    exit(0);
}