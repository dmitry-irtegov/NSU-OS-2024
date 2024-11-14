#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

void convert_to_upper_and_print(FILE *stream) {
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), stream) != NULL) {
        // Convert each character to uppercase
        for (int i = 0; buffer[i] != '\0'; i++) {
            buffer[i] = toupper(buffer[i]);
        }
        printf("Text in uppercase: %s", buffer);
    }
}

int main() {
    FILE *pipe_stream;

    // Open a new process, redirecting its output to a pipe
    pipe_stream = popen("echo 'Sample text with Mixed Case.'", "r");
    if (pipe_stream == NULL) {
        perror("Failed to open pipe via popen");
        return 1;
    }

    // Read data from the stream and convert it to uppercase
    convert_to_upper_and_print(pipe_stream);

    // Close the stream and check for successful child process termination
    if (pclose(pipe_stream) == -1) {
        perror("Error closing pipe via pclose");
        return 1;
    }

    return 0;
}
