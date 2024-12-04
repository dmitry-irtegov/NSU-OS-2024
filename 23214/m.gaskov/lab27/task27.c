#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int ends_with_newline(const char *str) {
    if (str == NULL) {
        return 0;
    }
    size_t len = strlen(str);
    if (len > 0 && str[len - 1] == '\n') {
        return 1;
    }
    return 0;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    FILE *file;
    if ((file = fopen(argv[1], "r")) == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    FILE *pipe;
    if ((pipe = popen("wc -l", "w")) == NULL) {
        perror("pipe");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    char *line = NULL;
    size_t len = 0;
    int last_line_is_empty = 1;

    while (getline(&line, &len, file) != -1) {
        last_line_is_empty = ends_with_newline(line);
        if (line[0] == '\n') {
            if (fputc('\n', pipe) == EOF) {
                perror("fputc");
                free(line);
                pclose(pipe);
                fclose(file);
                exit(EXIT_FAILURE);
            }
        }
    }
    if (last_line_is_empty == 1) {
        if (fputc('\n', pipe) == EOF) {
            perror("fputc");
            free(line);
            pclose(pipe);
            exit(EXIT_FAILURE);
        }
    }

    free(line);
    fclose(file);

    if (pclose(pipe) == -1) {
        perror("pclose");
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}
