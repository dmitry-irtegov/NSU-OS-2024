#include <stdio.h>
#include <stdlib.h>

int main() {
    char* string = "I'm just a simple English string";

    FILE* pipe;

    // if((pipe = popen("./toUpper", "w")))
    if ((pipe = popen("tr '[:lower:]' '[:upper:]'", "w")) == NULL) {
        perror("popen failed");
        return 1;
    }

    fputs(string, pipe);
    pclose(pipe);
    return 0;
}
