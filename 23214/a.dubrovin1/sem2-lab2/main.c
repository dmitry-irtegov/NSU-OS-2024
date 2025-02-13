#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

void* thread_body(void * param) {
        char* name = (char*) param;

        for (int i = 0; i < 10; i++) {
                fprintf(stdout, "I'm a %s thread, and it's line number %d\n", name, i);
        }

        return NULL;
}

int main() {
        pthread_t thread;
        int code;
        char parent[7] = "parent";
        char child[6] = "child";

        code = pthread_create(&thread, NULL, thread_body, (void *) child);

        if (code != 0) {
                char buf[256];
                strerror_r(code, buf, sizeof(buf));
                fprintf(stderr, "Unable to create a thread: %s\n", buf);
                exit(1);
        }

        pthread_join(thread, NULL);
        thread_body((void *) parent);
        exit(0);
}
