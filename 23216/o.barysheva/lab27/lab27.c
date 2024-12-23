#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <inputfile>\n", argv[0]);
        return EXIT_FAILURE;
    }

    FILE *input = fopen(argv[1], "r");
    if (!input) {
        perror("Error opening input file");
        return EXIT_FAILURE;
    }

    FILE *output = popen("wc -l", "w");
    if (!output) {
        perror("Error opening pipe to wc");
        fclose(input);
        return EXIT_FAILURE;
    }

    char *line = NULL;
    size_t n = 0;

    while (getline(&line, &n, input) != -1) {
        if (*line == '\n') {
            fputc('\n', output);
        }
    }

    free(line); 
    fclose(input);
    if (pclose(output) == -1) {
        perror("Error closing pipe");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}