#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>

int main() {
    char* string = "HeHe hAhA";

    FILE* pipe;

    if((pipe = popen("./makeUp", "w")))

    fputs(string, pipe);

    pclose(pipe);
    return 0;
}
