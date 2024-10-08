#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    FILE *inputFile;
    FILE *outputFile;
    char *line = NULL;
    size_t len = 0;
    
    if (argc != 2) {
        return 1;
    }

    inputFile = fopen(argv[1], "r");
    if (inputFile == NULL) {
        fprintf(stderr, "Failed to open file\n");
        return 1;
    }

    outputFile = popen("wc -l", "w");

    while (getline(&line, &len, inputFile) != -1) {
        if (line[0] == '\n') {
            fputs(line, outputFile);
        }
    }

    free(line);
    fclose(inputFile);
    pclose(outputFile);

    return 0;
}
