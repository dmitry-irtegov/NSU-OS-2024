#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAXLENGTH 8

int main(int argc, char *argv[])
{
    if(argc < 2) {
        char *cmd = malloc(strlen(argv[0] + 3));
        if(cmd == NULL) {
            perror("unable to allocate memory");
            return 2;
        }
        strcpy(cmd, argv[0]);	
        strcat(cmd, " w");
        FILE* fpt = popen(cmd, "r");
        if(fpt == NULL) {
            perror("unable to create pipe");
            return 1;
        }
        char buf[MAXLENGTH];
        while(fgets(buf, MAXLENGTH, fpt) != NULL) {
            int len = strlen(buf);
            for(int i = 0; i < len; i++) {
                buf[i] = toupper(buf[i]);
                printf("%c", buf[i]);
            }
        }
        printf("\n");
        pclose(fpt);
    } else {
        char buf[] = "Sample Text That Will Be Written inTo the pipe";
        if(printf("%s", buf) < 0) {
            fprintf(stderr,"unable to write into the pipe from child");
            return 3;
        }
    }
    return 0;
}
