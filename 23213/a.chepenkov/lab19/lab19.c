#define _FILE_OFFSET_BITS 64
#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <fnmatch.h>

int main() {
    DIR* directory;
    struct dirent* inp;
    char pattern[BUFSIZ];
    int flag = 0;
    fgets(pattern, BUFSIZ, stdin);

    if (pattern[strlen(pattern) - 1] == '\n') {
        pattern[strlen(pattern) - 1] = '\0';
    }

    directory = opendir(".");
    if (directory == NULL) {
        perror("Failed to open directory");
        return -1;
    }

    while (inp = readdir(directory)) {
        if (fnmatch(pattern, inp->d_name, 0) == 0) {
            printf("%s\n", inp->d_name);
            flag = 1;
        }
    }

    if (flag == 0) {
        printf("No files found with pattern: %s\n", pattern);
    }
    closedir(directory);
    return 0;
}
