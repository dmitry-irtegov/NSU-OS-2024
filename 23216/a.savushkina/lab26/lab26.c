#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int main() {
    char buf[20];
    FILE *pipe;

    pipe = popen("echo 'text for upper LOL'", "r");
    if (pipe == NULL) {
        perror("error in popen");
        exit(EXIT_FAILURE);
    }

    if (fgets(buf, sizeof(buf), pipe) == NULL) {
        perror("error in fgets");
        pclose(pipe);
        exit(EXIT_FAILURE);
    }

    if (pclose(pipe) == -1) {
        perror("error in pclose");
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < strlen(buf); i++) {
        buf[i] = toupper(buf[i]);
    }

    if (fputs(buf, stdout) == EOF) {
        perror("error in fputs");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}
