#include <stdlib.h>
#include <stdio.h>

int main(int argc, char** argv) 
{
    if (argc < 2) {
        return -1;
    }

    FILE* input = fopen(argv[1], "r");
    if (input == NULL) {
        perror("Can't open file");
        exit(EXIT_FAILURE);
    }

    FILE* output = popen("wc -m", "w");
    if (output == NULL) {
        perror("Can't open pipe");
        exit(EXIT_FAILURE);
    }

    char* str = NULL;
    int n = 0;

    while (getline(&str, &n, input) != -1) {
        if (str[0] == '\n') {
            fputc('a', output);
        }
    }

    if (str != NULL) {
        free(str);
    }

    fclose(input);
    pclose(output);

    return 0;
}
