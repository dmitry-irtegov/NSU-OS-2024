#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Correct usage: %s <file>\n", argv[0]);
        return 1;
    }

    FILE *input;
    FILE *pipe;

    if((pipe = popen("wc -l", "w")) == NULL) {
        perror("can not open pipe");
        return 1;
    }

    if((input = fopen(argv[1], "r")) == NULL) {
        perror("can not read input file");
        return 1;
    }

    char *line = NULL;        
    size_t len = 0;
    char prev_string_last_char;
    while(getline(&line, &len, input) != -1) {
        prev_string_last_char = line[strlen(line)-1];
        if(line[0] == '\n') {
            fputs(line, pipe);
        }
    }
    if(prev_string_last_char == '\n'){
        fputs("\n", pipe);
    }

    free(line);
    fclose(input);
    if (pclose(pipe) == -1){
        perror("pclose fail");
        return 1;
    }
    return 0;



}
