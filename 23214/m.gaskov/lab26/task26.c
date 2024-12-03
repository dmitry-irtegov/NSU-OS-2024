#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    FILE *pipe;
    char *text = "Hello, WoRLd!\n";
    if ((pipe = popen("./my_tr", "w")) == NULL) {
        perror("popen");
        exit(EXIT_FAILURE);
    }
    if (fwrite(text, sizeof(char), strlen(text), pipe) != strlen(text)) {
        perror("fwrite");
        pclose(pipe);
        exit(EXIT_FAILURE);
    }
    if (pclose(pipe) == -1) {
        perror("pclose");
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}
