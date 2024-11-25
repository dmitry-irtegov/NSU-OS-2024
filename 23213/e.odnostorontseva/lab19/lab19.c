#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <fnmatch.h>

int main(int argc, char* argv[]) {

    if(argc < 2) {
        fprintf(stderr, "Usage: %s <<template>>. \n", argv[0]);
        return 1;
    }

    if (strchr(argv[1], '/') != NULL) {
        fprintf(stderr, "'/' character cannot be in the template. \n");
        return 1;
    }

    DIR* dir;
    if ((dir = opendir(".")) == NULL) {
        perror("opendir error");
        return 1;
    }
    
    struct dirent* dp;
    int match = 0;
    errno = 0;

    while ((dp = readdir(dir)) != NULL) {
        int res = fnmatch(argv[1], dp->d_name, FNM_PATHNAME);
        if (res == 0) {
            printf("%s\n", dp->d_name);
            match = 1;
        } else if (res != FNM_NOMATCH) {
            fprintf(stderr, "error");
            closedir(dir);
            return 1;
        }
    }

    if (errno != 0) {
        perror("readdir error");
        closedir(dir);
        return 1;
    }

    if (match == 0) {
        printf("No files match template: %s\n", argv[1]);
    }
    
    closedir(dir);
    return 0;
}

