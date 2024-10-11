#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#define BATCHSIZE 1024

int main(int argc, char* argv[]) {
    
    if (argc < 2) {
        fprintf(stderr, "Wrong passing of arguments\n");
        return 1;
    }

    FILE *fp;
    char cmd[1050];
    snprintf(cmd, sizeof(cmd), "cat '%s'", argv[1]);

    fp = popen(cmd, "r");
    if (fp == NULL) {
        perror("popen error");
        return 1;
    }
    char buffer[BATCHSIZE];
    size_t bytes_read;

    while ((bytes_read = fread(buffer, sizeof(char), BATCHSIZE, fp)) > 0) {
        if (ferror(fp)) {
            perror("fread error");
            pclose(fp);
            return 1;
        }
        for (int i = 0; i < bytes_read; i++) {
            printf("%c", toupper(buffer[i]));  
        }
    }
    printf("\n");


    pclose(fp);

    return 0;
}
