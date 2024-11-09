#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>


int main() {
    int size = 10;
    char str[size];
    fgets(str, 10, stdin);
    

    for(int i = 0; i < size - 1; i++) {
        printf("%c", toupper(str[i]));
    }
    printf("\n");
}
