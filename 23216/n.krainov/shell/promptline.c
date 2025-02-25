#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "shell.h"

void getPrompt(char* prompt){
    sprintf(prompt,"%s$ ", getcwd(NULL, 100));
}

int promptline(char *line, int sizline) {
    int n = 0;
    char prompt[100];
    getPrompt(prompt);

    write(1, prompt, strlen(prompt));
    while (1) {
        n += read(0, (line + n), sizline-n);
        *(line+n) = '\0';

        if (*(line+n-2) == '\\' && *(line+n-1) == '\n') {
            *(line+n) = ' ';
            *(line+n-1) = ' ';
            *(line+n-2) = ' ';
            continue;   /*  read next line  */
        }
        return n;      /* all done */
    }
}