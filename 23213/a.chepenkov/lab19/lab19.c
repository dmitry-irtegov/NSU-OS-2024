#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <fnmatch.h>
#include <errno.h>

int main() {
    DIR* directory;
    struct dirent* inp;
    char pattern[BUFSIZ];
    int flag = 0;
    int fnmatch_res = 0;
    fgets(pattern, BUFSIZ, stdin);

    if (pattern[strlen(pattern) - 1] == '\n') {
        pattern[strlen(pattern) - 1] = '\0';
    }

    directory = opendir(".");
    if (directory == NULL) {
        perror("Failed to open directory");
        return -1;
    }

    while (1) {
	inp = readdir(directory);
	if (inp == NULL && errno != 0) {
	    perror("An error occured while readdir function");
	    return -1;
	}
	else if (inp == NULL) {
	    break;
	}
        fnmatch_res = fnmatch(pattern, inp->d_name, 0);
        if (fnmatch_res == 0) {
            printf("%s\n", inp->d_name);
            flag = 1;
        }
        else if (fnmatch_res != FNM_NOMATCH) {
            fprintf(stderr, "An error occured while fnmatch");
        }
    }

    if (flag == 0) {
        printf("No files found with pattern: %s\n", pattern);
    }
    closedir(directory);
    return 0;
}
