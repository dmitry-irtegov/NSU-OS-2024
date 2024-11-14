#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("There must be only the inputfile name.\n");
        return EXIT_FAILURE;
    }

    FILE *input = fopen(argv[1], "r");
    if (input == NULL) {
        perror("Error while opening file");
        return EXIT_FAILURE;
    }

    FILE *output = popen("wc -l", "w");
    if (output == NULL) {
        perror("Error with popen()");
        return EXIT_FAILURE;
    }

    char *line = NULL;
    size_t n = 0;
    while (getline(&line, &n, input) != -1) {
        if (line[0] == '\n') {
            fputc('\n', output);
        }
    }

    if (line != NULL) {
        free(line);
    }

    fclose(input);
    pclose(output);
    return EXIT_SUCCESS;
}