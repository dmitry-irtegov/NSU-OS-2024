#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "queue.h"

int main() {
    queue queue;
    mymsginit(&queue);

    printf("Main: Calling mymsgdrop.\n");
    mymsgdrop(&queue);

    printf("Main: Calling mymsgdestroy.\n");
    mymsgdestroy(&queue);

    printf("Main: Exiting.\n");
    return 0;
}