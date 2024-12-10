#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv){
    if (argc != 2){
        perror("ERROR: wrong format of start! Try ./lab27 <file>");
        exit(EXIT_FAILURE);
    }

    FILE* pipe = popen("wc -l", "w");
    if (!pipe){
        perror("ERROR: failed to open pipe!");
        exit(EXIT_FAILURE);
    }
    FILE* file = fopen(argv[1], "r");
    if (!file){
        perror("ERROR: failed to open file");
        pclose(pipe);
        exit(EXIT_FAILURE);
    }

    char* str = NULL;
    size_t n;
    while (getline(&str, &n, file) != -1){
        if (str[0] == '\n'){
            if (fputc('\n', pipe) == EOF){
                perror("ERROR: fputc failed!");
                free(str);
                fclose(file);
                pclose(pipe);
                exit(EXIT_FAILURE);
            }
        }
    }

    free(str);
    fclose(file);
    pclose(pipe);
    exit(EXIT_SUCCESS);
}