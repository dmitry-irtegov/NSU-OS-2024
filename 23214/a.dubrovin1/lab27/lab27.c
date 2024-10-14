#include <stdio.h>

int main(int argc, char** argv){
    FILE* input;
    FILE* output;

    if (argc != 2){
        fprintf(stderr, "inappropriate number of arguments\n");
        return 0;
    }

    input = fopen(argv[1], "r");
    if (input == NULL){
        fprintf(stderr, "Cannot open inputfile\n");
        return 0;
    }

    output = popen("wc -l", "w");
    if (output == NULL){
        fprintf(stderr, "Cannot open pipe\n");
        return 0;
    }

    char prevChar, currChar;
    prevChar = '\n';

    while ((currChar = fgetc(input)) != EOF){
        if (currChar == '\n' && prevChar == '\n'){
            putc(currChar, output);
        }

        prevChar = currChar;
    }

    fclose(input);
    pclose(output);
    return 0;
}
