#include <glob.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>

int errfunc(const char* msg, int c) {
    if (c != ENOTDIR) {
        fprintf(stderr, "%s: %s\n", msg, strerror(c));
    }
    return 0;
}

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "Wrong amount of arguments for program\n");
        return -1;
    }
    glob_t storage;
    char* buffer = argv[1];
    char buffer2[strlen(buffer) * 2];
    
    if (buffer[strlen(buffer) - 1] == '\n') {
        buffer[strlen(buffer) - 1] = '\0';
    }

    int pos = 0;

    for (size_t i = 0; i <= strlen(buffer); i++)
    {
        if (buffer[i] == '[' || buffer[i] == ']') {
            buffer2[pos++] = '\\';
        }        
        buffer2[pos++] = buffer[i];
    }
    

    if (glob(buffer2, 0, &errfunc, &storage) == GLOB_NOMATCH) {
        printf("No files matched with pattern %s\n", buffer);
        return -1;
    }

    for (size_t i = 0; i < storage.gl_pathc; i++) {
        printf("%s\n", storage.gl_pathv[i]);
    }    
    globfree(&storage);
    return 0;
}
