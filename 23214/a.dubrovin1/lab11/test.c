#include <stdio.h>
#include "execvpe.h"

int main(int argc, char** argv){
    if(argc < 2){
        fprintf(stderr, "Not enough arguments\n");
        return 1;
    }

    char* newenw[] = {"PATH=/usr/bin:/bin", "USER=a.plyusnin", NULL};
    execvpe(argv[1], &argv[1], newenw);

    perror("execvpe finished with error");
    return 1;
}
