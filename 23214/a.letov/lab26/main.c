#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

void main(void){
    char buffer[5];
    FILE* fpopen = popen("echo TestIngerkadzerbumadfagwgnaig", "r");
    if(fpopen==NULL){
        perror("popen error");
        exit(1);
    }
    while(fgets(buffer, 5, fpopen)!=NULL){
        size_t size = strlen(buffer);
        for(int i = 0;i<size;i++){
            buffer[i]=toupper(buffer[i]);
        }
        printf("%s", buffer);
    }
    if(ferror(fpopen)!=0){
        perror("fgets error");
        exit(2);
    }
    if(pclose(fpopen)==-1){
        perror("pclose error");
        exit(3);
    }
    exit(0);
}