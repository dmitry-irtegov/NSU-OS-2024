#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Correct usage: %s <file>\n", argv[0]);
        return 1;
    }
    FILE *pipe;

    char command[256];
    snprintf(command, sizeof(command), "cat %s", argv[1]);
    if((pipe = popen(command, "r")) == NULL) {
        perror("can not open pipe");
        return 1;
    }

    char *line = NULL;
    size_t len = 0;
    while(getline(&line, &len, pipe) != -1) {
        int i;
        for (i = 0; line[i] != '\0'; i++) {
            char c = toupper(line[i]);
            printf("%c", c);
        }

    }

    free(line);
    if (pclose(pipe) == -1){
        perror("pclose fail");
        return 1;
    }
    return 0;
}
