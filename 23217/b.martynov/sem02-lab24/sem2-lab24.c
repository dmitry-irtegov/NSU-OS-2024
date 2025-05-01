#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <synch.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#define SLEEP_A 1
#define SLEEP_B 2
#define SLEEP_C 3

sema_t semaA;
sema_t semaB;
sema_t semaC;
sema_t semaModule;

void cleaner(int signum) {
    char buf[256]; 
    int me = 0;
    if ((me = sema_destroy(&semaA)) != 0) {
        strerror_r(me, buf, sizeof(buf));
        fprintf(stderr, "SemaA destory error: %s\n", buf);
        exit(EXIT_FAILURE);
    }
    printf("A destroyed\n");
    if ((me = sema_destroy(&semaB)) != 0) {
        strerror_r(me, buf, sizeof(buf));
        fprintf(stderr, "SemaB destory error: %s\n", buf);
        exit(EXIT_FAILURE);
    }
    printf("B destroyed\n");
    if ((me = sema_destroy(&semaC)) != 0) {
        strerror_r(me, buf, sizeof(buf));
        fprintf(stderr, "SemaC destory error: %s\n", buf);
        exit(EXIT_FAILURE);
    }
    printf("C destroyed\n");
    if ((me = sema_destroy(&semaModule)) != 0) {
        strerror_r(me, buf, sizeof(buf));
        fprintf(stderr, "SemaModule destory error: %s\n", buf);
        exit(EXIT_FAILURE);
    }
    printf("M destroyed\n");

    exit(EXIT_SUCCESS);
}

void* createA(void* param) {
    char buf[256]; 
    int me = 0;
    while (1) {
        (void) sleep(SLEEP_A);
        printf("\tA\n");

        if ((me = sema_post(&semaA)) != 0) {
            strerror_r(me, buf, sizeof(buf));
            fprintf(stderr, "SemaA post error: %s\n", buf);
            exit(EXIT_FAILURE);
        }
    }
}

void* createB(void* param) {
    char buf[256]; 
    int me = 0;
    while (1) {
        (void) sleep(SLEEP_B);
        printf("\t\tB\n");

        if ((me = sema_post(&semaB)) != 0) {
            strerror_r(me, buf, sizeof(buf));
            fprintf(stderr, "SemaB post error: %s\n", buf);
            exit(EXIT_FAILURE);
        }
    }
}

void* createC(void* param) {
    char buf[256]; 
    int me = 0;
    while (1) {
        (void) sleep(SLEEP_C);
        printf("\t\t\tC\n");

        if ((me = sema_post(&semaC)) != 0) {
            strerror_r(me, buf, sizeof(buf));
            fprintf(stderr, "SemaC post error: %s\n", buf);
            exit(EXIT_FAILURE);
        }
    }
}

void* createModule(void* parampampam) {
    char buf[256]; 
    int me = 0;
    while (1) {
        if ((me = sema_wait(&semaA)) != 0) {
            strerror_r(me, buf, sizeof(buf));
            fprintf(stderr, "SemaA wait error: %s\n", buf);
            exit(EXIT_FAILURE);
        }
        if ((me = sema_wait(&semaB)) != 0) {
            strerror_r(me, buf, sizeof(buf));
            fprintf(stderr, "SemaB wait error: %s\n", buf);
            exit(EXIT_FAILURE);
        }

        if ((me = sema_post(&semaModule)) != 0) {
            strerror_r(me, buf, sizeof(buf));
            fprintf(stderr, "SemaModule post error: %s\n", buf);
            exit(EXIT_FAILURE);
        }
        printf("\t\t\t\tM\n");
    }
}

int main() {
    char buf[256]; 
    int me = 0;

    if ((me = sema_init(&semaA, 0, USYNC_THREAD, NULL)) != 0) {
        strerror_r(me, buf, sizeof(buf));
        fprintf(stderr, "Init semaA error: %s\n", buf);
        exit(EXIT_FAILURE);
    }
    if ((me = sema_init(&semaB, 0, USYNC_THREAD, NULL)) != 0) {
        strerror_r(me, buf, sizeof(buf));
        fprintf(stderr, "Init semaB error: %s\n", buf);
        exit(EXIT_FAILURE);
    }
    if ((me = sema_init(&semaC, 0, USYNC_THREAD, NULL)) != 0) {
        strerror_r(me, buf, sizeof(buf));
        fprintf(stderr, "Init semaC error: %s\n", buf);
        exit(EXIT_FAILURE);
    }
    if ((me = sema_init(&semaModule, 0, USYNC_THREAD, NULL)) != 0) {
        strerror_r(me, buf, sizeof(buf));
        fprintf(stderr, "Init semaModule error: %s\n", buf);
        exit(EXIT_FAILURE);
    }

    signal(SIGINT, cleaner);

    // =====

    pthread_t threadA;
    pthread_t threadB;
    pthread_t threadC;
    pthread_t threadModule;
    if ((me = pthread_create(&threadA, NULL, createA, NULL)) != 0) {
        strerror_r(me, buf, sizeof(buf));
        fprintf(stderr, "Creating threadA error: %s\n", buf);
        exit(EXIT_FAILURE);
    }
    if ((me = pthread_create(&threadB, NULL, createB, NULL)) != 0) {
        strerror_r(me, buf, sizeof(buf));
        fprintf(stderr, "Creating threadB error: %s\n", buf);
        exit(EXIT_FAILURE);
    }
    if ((me = pthread_create(&threadC, NULL, createC, NULL)) != 0) {
        strerror_r(me, buf, sizeof(buf));
        fprintf(stderr, "Creating threadC error: %s\n", buf);
        exit(EXIT_FAILURE);
    }
    if ((me = pthread_create(&threadModule, NULL, createModule, NULL)) != 0) {
        strerror_r(me, buf, sizeof(buf));
        fprintf(stderr, "Creating threadC error: %s\n", buf);
        exit(EXIT_FAILURE);
    }

   
    while (1) {
        if ((me = sema_wait(&semaC)) != 0) {
            strerror_r(me, buf, sizeof(buf));
            fprintf(stderr, "SemaC wait error: %s\n", buf);
            exit(EXIT_FAILURE);
        }
        if ((me = sema_wait(&semaModule)) != 0) {
            strerror_r(me, buf, sizeof(buf));
            fprintf(stderr, "SemaModule wait error: %s\n", buf);
            exit(EXIT_FAILURE);
        }

        printf("Widget!\n");
    }
    
    exit(EXIT_SUCCESS);
}
