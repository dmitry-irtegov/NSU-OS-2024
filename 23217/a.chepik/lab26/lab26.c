#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <ctype.h>
#define STRING_SIZE 12

int main() {
    char read_string[STRING_SIZE];

    FILE* pipe = popen("./toupper", "r");
    if (!pipe) {
        printf("popen returned NULL.");
        exit(-1);
    }
    
    if (fgets(read_string, STRING_SIZE, pipe) == NULL) {
        printf("Error reading file.\n");
        pclose(pipe);
        exit(-1);
    }

    for (int i = 0; i < STRING_SIZE; i++) {
        printf("%c", toupper(read_string[i]));
    }

    printf("\n");

    pclose(pipe);
    exit(0);
}
