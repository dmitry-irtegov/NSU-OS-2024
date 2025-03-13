#include <pthread.h>
#include <stdio.h>

char flag = 1;
pthread_mutex_t mutex;

int tryToPrint(char* text, int i) {
    if (pthread_mutex_lock(&mutex)) {
        return 1;
    }

    printf("%s %d\n", text, i);

    pthread_mutex_unlock(&mutex);

    return 0;
}

void* threadFunc(void* ignored) {
    int count = 0;

    while (count < 11) {
        if (flag == 0) {
            if (tryToPrint("thread text", count) == 1) {
                continue;
            }

            flag = 1;
            count++;
        }
    }
}

int main() {
    pthread_t thread;

    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
    pthread_mutex_init(&mutex, &attr);
    pthread_create(&thread, NULL, threadFunc, NULL);

    int count = 0;

    while (count < 11) {
        if (flag == 1) {
            if (tryToPrint("main text", count) == 1) {
                continue;
            }

            count++;
            flag = 0;
        }
    }

    pthread_exit(NULL);
}
