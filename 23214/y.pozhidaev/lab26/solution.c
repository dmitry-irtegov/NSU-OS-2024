#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

int main() {
    FILE *pipe;
    char text[BUFSIZ];

    if( (pipe = popen("echo HelLo WorLd aNd evERyone!", "r")) == NULL){
        perror("popen error");
        exit(1);
    }

    while(fgets(text,BUFSIZ,pipe) != NULL){
        for (int i = 0; text[i] != 0; i++){
            text[i] = toupper(text[i]);
        }
        printf("%s", text);
    }

    if (ferror(pipe) > 0){
        perror("fgets error");
        exit(2);
    }

    if((pclose(pipe)) == -1 ){
        perror("pclose error");
        exit(3);
    }
    exit(0);
}

