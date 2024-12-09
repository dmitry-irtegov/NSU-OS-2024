#include <glob.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

int errfunc(const char* msg, int c) {
    if (c != ENOTDIR) {
        fprintf(stderr, "%s: %s\n", msg, strerror(c));
    }
    return 0;
}

int main() {
    glob_t storage;
    char buffer[BUFSIZ];
    fgets(buffer, BUFSIZ, stdin);
    
    if (glob(buffer, 0, &errfunc, &storage) == GLOB_NOMATCH) {
        printf("No files matched with pattern %s\n", buffer);
        return -1;
    }

    for (size_t i = 0; i < storage.gl_pathc; i++) {
        printf("%s\n", storage.gl_pathv[i]);
    }    
    globfree(&storage);
    return 0;
}
