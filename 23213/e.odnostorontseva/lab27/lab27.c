#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[]) {

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    FILE *input_file, *output_file;
    char line[BUFSIZ];

    input_file = fopen(argv[1], "r");

    if (input_file == (FILE*)NULL) {
        perror("fopen error");
        return 1;
    }

    output_file = popen("wc -l", "w");

    if (output_file == (FILE*)NULL) {
        perror("popen error");
        fclose(input_file);
        return 1;
    }

    char last = '\n';
    while (fgets(line, BUFSIZ, input_file) != (char *)NULL) {
        if (line[0] == '\n' && last == '\n') {
            fputs(line, output_file);
        }
        last = line[strlen(line) - 1];
    }
   
    fclose(input_file);
    pclose(output_file);
    return 0;
}

