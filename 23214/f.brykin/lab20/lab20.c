#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glob.h>
#include <errno.h>

int main() {
    char pattern[BUFSIZ];
    glob_t glob_struct;
    int res;
    printf("Enter file name pattern: ");
    if (fgets(pattern, BUFSIZ, stdin) == NULL) {
        perror("Error reading pattern");
        return EXIT_FAILURE;
    }
    size_t len = strlen(pattern);
    if (len > 0 && pattern[len - 1] == '\n') {
        pattern[len - 1] = '\0';
    }
    res = glob(pattern, 0, NULL, &glob_struct);
    if (res == GLOB_NOMATCH) {
        printf("No files found matching the pattern: %s\n", pattern);
        return EXIT_FAILURE;
    }
    char** pattern_matched = glob_struct.gl_pathv;
    while (*pattern_matched) {
        printf("%s\n", *pattern_matched);
        pattern_matched++;
    }
    globfree(&glob_struct);
    return EXIT_SUCCESS;
}
