#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv){
    if (argc != 2){
        fprintf(stderr, "ERROR: wrong format to start! Try %s <file>", argv[0]);
        exit(EXIT_FAILURE);
    }

    FILE* file = fopen(argv[1], "r");
    if (!file){
        perror("ERROR: failed to open file!");
        exit(EXIT_FAILURE);
    }
    FILE* pipe = popen("wc -l", "w");
    if (!pipe){
        perror("ERROR: failed to open pipe!");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    char* str = NULL;
    size_t strSize;
    while(getline(&str, &strSize, file) != -1){
        if (str[0] == '\n'){
            if (fputc(str[0], pipe) == -1){
                perror("ERROR: failed to write buffer into the pipe!");
                fclose(file);
                pclose(pipe);
                free(str);
                exit(EXIT_FAILURE);
            }
        }
    }

    fclose(file);
    pclose(pipe);
    free(str);
    exit(EXIT_SUCCESS);
}
