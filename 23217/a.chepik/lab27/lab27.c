#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
    if (argc != 2) {
        printf("File name is missing from argv.\n");
        exit(-1);
    }

    FILE* input = fopen(argv[1], "r");
    if (input == NULL) {
        printf("fopen() failed.\n");
        exit(-1);
    }

    FILE* output = popen("wc -m", "w");
    if (output == NULL) {
        printf("popen() failed.\n");
        exit(-1);
    }

    size_t bytes = 0;
    char* getline_result = NULL;

    while (getline(&getline_result, &bytes, input) != -1) {
        if (getline_result[0] == '\n') {
            fputc('n', output);
        }
    }

    if (getline_result != NULL) {
        free(getline_result);
    }

    fclose(input);
    pclose(output);

    exit(0);
}
