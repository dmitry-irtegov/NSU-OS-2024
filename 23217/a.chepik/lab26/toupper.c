#include <stdio.h>
#include <unistd.h>
#define STRING_SIZE 12

int main() {
    char test_string[STRING_SIZE] = "test_string";
    write(STDOUT_FILENO, test_string, STRING_SIZE);
}
