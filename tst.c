#include <stdio.h>
#include <unistd.h>

int main() {
    int seconds = 0;
    while (1) {
        printf("%d\n", seconds++);
        fflush(stdout);
        sleep(1);
    }
    return 0;
}
