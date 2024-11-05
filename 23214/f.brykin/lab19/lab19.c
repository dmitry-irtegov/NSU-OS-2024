#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <fnmatch.h>
#include <errno.h>

int main() {
    DIR *dir;
    struct dirent *entry;
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
    if (strchr(pattern, '/') != NULL) {
        fprintf(stderr, "Error: Pattern should not contain '/'\n");
        return EXIT_FAILURE;
    }
    int found = 0;
    dir = opendir(".");
    if (dir == NULL) {
        perror("Error opening directory");
        return EXIT_FAILURE;
    }
    while ((entry = readdir(dir)) != NULL) {
        if (fnmatch(pattern, entry->d_name, FNM_NOESCAPE) == 0) {
            printf("%s\n", entry->d_name);
            found = 1;
        } else if (errno != 0) {
            perror("Error matching file name");
            closedir(dir);
            return EXIT_FAILURE;
        }
    }
    if (!found) {
        printf("No files found matching the pattern: %s\n", pattern);
    }
    return EXIT_SUCCESS;
}