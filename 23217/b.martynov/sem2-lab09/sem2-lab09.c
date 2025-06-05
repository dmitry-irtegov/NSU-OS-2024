#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#define ITERS_CHECK 1000000

int cnt;
pthread_t* threads;
double* parts;
int flag = 0;

void SIGINTer(int sigNum) {
    printf("Ctrl+C\n");
    flag = 1;
}


void* childFunc(void* arg) {
    int a = (int)arg;

    double tmp = 0.0;
    long long i = a;
    while (1) {
        tmp += 1.0/(i*4.0 + 1.0);
        tmp -= 1.0/(i*4.0 + 3.0);
        if (flag && (i > ITERS_CHECK)) {
            break;
        }

        i += cnt;
    }

    parts[a] = tmp;

    return NULL;
}

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "Please set count of threads in first argument\n");
        exit(EXIT_FAILURE);
    }

    cnt = atoi(argv[1]);
    if (cnt == 0) {
        fprintf(stderr, "Please use integer number for count of threads\n");
        exit(EXIT_FAILURE);
    }

    if (signal(SIGINT, SIGINTer) == SIG_ERR) {
        perror("Error whiiile set SIGINT handler");
        exit(EXIT_FAILURE);
    }

    threads = (pthread_t*)malloc(cnt * sizeof(pthread_t));
    parts = (double*)malloc(cnt * sizeof(double));

    for (int i = 0; i < cnt; i++) {
        parts[i] = 0.0; // o.o
        int thrErr = pthread_create(threads + i, NULL, childFunc, (void*)i);
        if (thrErr != 0) {
            char buf[256];
            strerror_r(thrErr, buf, sizeof(buf));
            fprintf(stderr, "Creating thread error: %s\n", buf);
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < cnt; i++) {
        void* ret;
        int thrErr = pthread_join(threads[i], &ret);
        if (thrErr != 0) {
            char buf[256];
            strerror_r(thrErr, buf, sizeof(buf));
            fprintf(stderr, "Thread join error: %s\n", buf);
            exit(EXIT_FAILURE);
        }
    }

    double pi = 0.0;
    for (int i = 0 ; i < cnt; i++) {
        pi += parts[i];
    }
    pi *= 4;
    printf("pi done - %.15g \n", pi);

    free(threads);
    free(parts);

    return (EXIT_SUCCESS);
}
