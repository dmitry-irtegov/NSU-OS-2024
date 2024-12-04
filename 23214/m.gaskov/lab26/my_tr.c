#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>

#define BUFFER_SIZE 256

int main() {
    char buffer[BUFFER_SIZE];
    while (1) {
        ssize_t count = read(STDIN_FILENO, buffer, BUFFER_SIZE - 1);
        if (count == -1) {
            perror("read");
            exit(EXIT_FAILURE);
        } else if (count == 0) {
            break;
        }
        for (int i = 0; i < count; ++i) {
            buffer[i] = toupper(buffer[i]);
        }
        buffer[count] = '\0';
        printf("%s", buffer);
    }
    exit(EXIT_SUCCESS);
}
