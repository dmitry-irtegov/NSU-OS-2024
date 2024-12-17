#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

int main() {
    FILE *fp;
    char buf[BUFSIZ];

    fp = popen("cat", "r");
    if (fp == NULL) {
        perror("popen");
        exit(EXIT_FAILURE);
    }

    while (fgets(buf, sizeof(buf), fp) != NULL) {
        if (ferror(fp) != 0) {
            perror("fgets error");
            exit(EXIT_FAILURE);
  }
        for (int i = 0; buf[i]; i++) {
            buf[i] = toupper(buf[i]);
        }
        printf("%s", buf);
    }

    if (pclose(fp) == -1) {
        perror("pclose");
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}