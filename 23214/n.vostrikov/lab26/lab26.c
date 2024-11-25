#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#define SIZE 16

int main() {
    char buffer[SIZE];
    FILE* input = popen("cat text.txt", "r");
    if (input == NULL) {
        perror("popen failed");
        exit(EXIT_FAILURE);
    }
    while (fgets(buffer, SIZE, input) != NULL) {
        for (int i = 0; i < SIZE && buffer[i] != '\0'; ++i) {
            buffer[i] = toupper(buffer[i]);
        }
        printf("%s", buffer);
    }
    printf("\n");
    if(pclose(input) == -1) {
        perror("pclose failed");
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS); 
}
