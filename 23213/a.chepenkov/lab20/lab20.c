#include <stdio.h>
#include <string.h>
#include <glob.h>
#include <errno.h>

int errfunc(const char *epath, int eerrno) {
    if (eerrno != ENOTDIR) {
        fprintf(stderr, "%s: %s\n", epath, strerror(eerrno));
    }
    return 0;
}

int main() {
    glob_t glob_struct;
    int glob_res;
    char** matched;
    char pattern[BUFSIZ];
    fgets(pattern, BUFSIZ, stdin);

    if (pattern[strlen(pattern) - 1] == '\n') {
        pattern[strlen(pattern) - 1] = '\0';
    }

    glob_res = glob(pattern, 0, &errfunc, &glob_struct);
    if (glob_res == GLOB_NOMATCH) {
        printf("No files found with pattern: %s\n", pattern);
        return -1;
    }

    matched = glob_struct.gl_pathv;
    while (*matched) {
        printf("%s\n", *matched);
        matched++;
    }
    globfree(&glob_struct);
    return 0;
}
