#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

pthread_mutex_t p;
pthread_mutex_t ch;
pthread_mutex_t ru;

int flag = 0;

void* childFunc(void* param) {
    char buf[256]; 
    int me;
    if ((me = pthread_mutex_lock(&ch)) != 0) {
        strerror_r(me, buf, sizeof(buf));
        fprintf(stderr, "Lock ch error: %s\n", buf);
        exit(EXIT_FAILURE);
    }

    flag = 1;

    for (int i = 9; i > 0; i--) {
        if ((me = pthread_mutex_lock(&p)) != 0) {
            strerror_r(me, buf, sizeof(buf));
            fprintf(stderr, "Lock p error: %s\n", buf);
            exit(EXIT_FAILURE);
        }

        printf("\t%d.. (child)\n", i);

        if ((me = pthread_mutex_unlock(&ch)) != 0) {
            strerror_r(me, buf, sizeof(buf));
            fprintf(stderr, "Unlock ch error: %s\n", buf);
            exit(EXIT_FAILURE);
        }

        if ((me = pthread_mutex_lock(&ru)) != 0) {
            strerror_r(me, buf, sizeof(buf));
            fprintf(stderr, "Lock ru error: %s\n", buf);
            exit(EXIT_FAILURE);
        }

        if ((me = pthread_mutex_unlock(&p)) != 0) {
            strerror_r(me, buf, sizeof(buf));
            fprintf(stderr, "Unlock p error: %s\n", buf);
            exit(EXIT_FAILURE);
        }
//
        if ((me = pthread_mutex_lock(&ch)) != 0) {
            strerror_r(me, buf, sizeof(buf));
            fprintf(stderr, "Lock ch error: %s\n", buf);
            exit(EXIT_FAILURE);
        }
//
        if ((me = pthread_mutex_unlock(&ru)) != 0) {
            strerror_r(me, buf, sizeof(buf));
            fprintf(stderr, "Unlock ru error: %s\n", buf);
            exit(EXIT_FAILURE);
        }
//
    }

    if ((me = pthread_mutex_unlock(&ch)) != 0) {
        strerror_r(me, buf, sizeof(buf));
        fprintf(stderr, "Unlock ch error: %s\n", buf);
        exit(EXIT_FAILURE);
    }

    printf("\tChild puuusk!\n");
    pthread_exit(NULL);
}

int main() {
    char buf[256]; 
    int me;

    pthread_mutexattr_t attr;
    if ((me = pthread_mutexattr_init(&attr)) != 0) {
        strerror_r(me, buf, sizeof(buf));
        fprintf(stderr, "Mutex attr init error: %s\n", buf);
        exit(EXIT_FAILURE);
    }
    if ((me = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK)) != 0) {
        strerror_r(me, buf, sizeof(buf));
        fprintf(stderr, "Mutex type set error: %s\n", buf);
        exit(EXIT_FAILURE);
    }

    if ((me = pthread_mutex_init(&p, &attr) != 0)) {
        strerror_r(me, buf, sizeof(buf));
        fprintf(stderr, "Mutex p init error: %s\n", buf);
        exit(EXIT_FAILURE);
    }
    if ((me = pthread_mutex_init(&ch, &attr) != 0)) {
        strerror_r(me, buf, sizeof(buf));
        fprintf(stderr, "Mutex ch init error: %s\n", buf);
        exit(EXIT_FAILURE);
    }
    if ((me = pthread_mutex_init(&ru, &attr) != 0)) {
        strerror_r(me, buf, sizeof(buf));
        fprintf(stderr, "Mutex ru init error: %s\n", buf);
        exit(EXIT_FAILURE);
    }

    // =====

    if ((me = pthread_mutex_lock(&p)) != 0) {
        strerror_r(me, buf, sizeof(buf));
        fprintf(stderr, "Lock p error: %s\n", buf);
        exit(EXIT_FAILURE);
    }

    pthread_t thread;
    int thrErr = pthread_create(&thread, NULL, childFunc, NULL);
    if (thrErr != 0) {
        char buf[256];
        strerror_r(thrErr, buf, sizeof(buf));
        fprintf(stderr, "Creating thread error: %s\n", buf);
        exit(EXIT_FAILURE);
    }

    while (flag == 0) {
        // wait..
    }

    for (int i = 9; i > 0; i--) {
        printf("%d..\n", i);
        
        if ((me = pthread_mutex_lock(&ru)) != 0) {
            strerror_r(me, buf, sizeof(buf));
            fprintf(stderr, "Lock ru error: %s\n", buf);
            exit(EXIT_FAILURE);
        }

        if ((me = pthread_mutex_unlock(&p)) != 0) {
            strerror_r(me, buf, sizeof(buf));
            fprintf(stderr, "Unlock p error: %s\n", buf);
            exit(EXIT_FAILURE);
        }

        if ((me = pthread_mutex_lock(&ch)) != 0) {
            strerror_r(me, buf, sizeof(buf));
            fprintf(stderr, "Lock ch error: %s\n", buf);
            exit(EXIT_FAILURE);
        }

        if ((me = pthread_mutex_unlock(&ru)) != 0) {
            strerror_r(me, buf, sizeof(buf));
            fprintf(stderr, "Unlock ru error: %s\n", buf);
            exit(EXIT_FAILURE);
        }

        if ((me = pthread_mutex_lock(&p)) != 0) {
            strerror_r(me, buf, sizeof(buf));
            fprintf(stderr, "Lock p error: %s\n", buf);
            exit(EXIT_FAILURE);
        }

        if ((me = pthread_mutex_unlock(&ch)) != 0) {
            strerror_r(me, buf, sizeof(buf));
            fprintf(stderr, "Unlock ch error: %s\n", buf);
            exit(EXIT_FAILURE);
        }
    }
    
    if ((me = pthread_mutex_unlock(&p)) != 0) {
        strerror_r(me, buf, sizeof(buf));
        fprintf(stderr, "Unlock p error: %s\n", buf);
        exit(EXIT_FAILURE);
    }

    void* ret;
    thrErr = pthread_join(thread, &ret);
    if (thrErr != 0) {
        char buf[256];
        strerror_r(thrErr, buf, sizeof(buf));
        fprintf(stderr, "Thread join error: %s\n", buf);
        exit(EXIT_FAILURE);
    }

    printf("Parent wiiie!\n");

    if ((me = pthread_mutex_destroy(&p)) != 0) {
        strerror_r(me, buf, sizeof(buf));
        fprintf(stderr, "Mutex p destory error: %s\n", buf);
        exit(EXIT_FAILURE);
    }
    if ((me = pthread_mutex_destroy(&ch)) != 0) {
        strerror_r(me, buf, sizeof(buf));
        fprintf(stderr, "Mutex p destory error: %s\n", buf);
        exit(EXIT_FAILURE);
    }
    if ((me = pthread_mutex_destroy(&ru)) != 0) {
        strerror_r(me, buf, sizeof(buf));
        fprintf(stderr, "Mutex p destory error: %s\n", buf);
        exit(EXIT_FAILURE);
    }

    if ((me = pthread_mutexattr_destroy(&attr)) != 0) {
        strerror_r(me, buf, sizeof(buf));
        fprintf(stderr, "Mutex attr destory error: %s\n", buf);
        exit(EXIT_FAILURE);
    }
    
    exit(EXIT_SUCCESS);
}
