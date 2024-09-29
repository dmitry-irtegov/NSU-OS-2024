#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>

int match_pattern_filename(char* pattern, char* name) {
    char* p = pattern;
    char* n = name;
    while (*p != '\0') {
        if (*p == '*') {
            p++;
            while (*n != '\0') {
                if (match_pattern_filename(p, n) == 1) {
                    return 1;
                }
                n++;
            }
        }
        else if (*p == '?') {
            if (*n == '\0') {
                return 0;
            }
            p++;
            n++;
        }
        else if (*p == *n) {
            p++;
            n++;
        }
        else {
            return 0;
        }
    }
    return *n == '\0';
}


int main() {
    DIR* directory;
    struct dirent* inp;
    char pattern[BUFSIZ];
    int flag = 0;
    fgets(pattern, BUFSIZ, stdin);

    if (pattern[strlen(pattern) - 1] == '\n') {
        pattern[strlen(pattern) - 1] = '\0';
    }

    directory = opendir("./");
    if (directory == NULL) {
        perror("Failed to open directory");
        return -1;
    }

    while (inp = readdir(directory)) {
        if (match_pattern_filename(pattern, inp->d_name) == 1) {
            printf("%s\n", inp->d_name);
            flag = 1;
        }
    }

    if (flag == 0) {
        printf("No files found\n");
    }
    closedir(directory);
    return 0;
}
