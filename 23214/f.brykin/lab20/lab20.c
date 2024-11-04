#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <fnmatch.h>
#include <errno.h>
#include <sys/stat.h>

void search_directory(const char *dir_path, const char *pattern) {
    DIR *dir;
    struct dirent *entry;
    dir = opendir(dir_path);
    if (dir == NULL) {
        if (errno == EACCES) {
            static int reported = 0;
            if (!reported) {
                fprintf(stderr, "Permission denied: %s\n", dir_path);
                reported = 1;
            }
        } else {
            perror("Error opening directory");
        }
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            char full_path[BUFSIZ];
            snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);
            if (fnmatch(pattern, full_path, FNM_NOESCAPE) == 0) {
                printf("%s\n", full_path);
            }
            struct stat st;
            if (stat(full_path, &st) == 0 && S_ISDIR(st.st_mode)) {
                search_directory(full_path, pattern);
            }
        }
    }
}

int main() {
    char pattern[BUFSIZ];
    printf("Enter file name pattern: ");
    if (fgets(pattern, BUFSIZ, stdin) == NULL) {
        perror("Error reading pattern");
        return EXIT_FAILURE;
    }
    size_t len = strlen(pattern);
    if (len > 0 && pattern[len - 1] == '\n') {
        pattern[len - 1] = '\0';
    }
    if (pattern[0] == '/') {
        search_directory("/", pattern);
    } else {
        search_directory(".", pattern);
    }
    return EXIT_SUCCESS;
}