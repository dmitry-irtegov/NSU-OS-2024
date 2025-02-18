#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <limits.h>

#define num_steps 200000ull

pthread_mutex_t mutex_piece, mutex_result;
pthread_cond_t cond;
unsigned long long currentStart = 0;
int num_result;
char flag = 0;

void my_perror(char* text, int code) {
    fprintf(stderr, "%s: %s \n", text, strerror(code));
}

void handler() {
    int code;
    if ((code = pthread_mutex_lock(&mutex_piece))) {
        my_perror("pthread_mutex_lock", code);
        exit(EXIT_FAILURE);
    }
    flag = 1;
    if ((code = pthread_mutex_unlock(&mutex_piece))) {
        my_perror("pthread_mitex_unlock", code);
        exit(EXIT_FAILURE);
    }
}

int getNextPiece(unsigned long long* start, unsigned long long* end) {
    int code;
    if ((code = pthread_mutex_lock(&mutex_piece))) {
        my_perror("pthread_mutex_lock", code);
        exit(EXIT_FAILURE);
    }
    if (flag) {
        if ((code = pthread_mutex_unlock(&mutex_piece))) {
            my_perror("pthread_mutex_unlock", code);
            exit(EXIT_FAILURE);
        }
        return 1;
    }

    if (ULLONG_MAX - num_steps < currentStart) {
        if ((code = pthread_mutex_unlock(&mutex_piece))) {
            my_perror("pthread_mutex_unlock", code);
            exit(EXIT_FAILURE);
        }
        return 1;
    }

    *start = currentStart;
    *end = currentStart + num_steps;
    currentStart += num_steps;
    if ((code = pthread_mutex_unlock(&mutex_piece))) {
        my_perror("pthread_mutex_unlock", code);
        exit(EXIT_FAILURE);
    }
    return 0;
}

void* calc_series(void* ignored) {

    double* res = malloc(sizeof(double));

    if (res == NULL) {
        pthread_exit(NULL);
    }


    unsigned long long start, end;
    *res = 0;
    while (1) {
        if (getNextPiece(&start, &end) == 1) {
            pthread_exit((void*) res);
        }

        for (unsigned long long i = start; i < end; i++) {    
            *res += 1.0/(i*4.0 + 1.0);
            *res -= 1.0/(i*4.0 + 3.0);
        }
    }

    pthread_exit(NULL);
}

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Programm needs count of threads as a second arg\n");
        exit(EXIT_FAILURE);
    }

    int code;
    int count_threads = atoi(argv[1]);
    if (count_threads < 1) {
        fprintf(stderr, "incorrect count of threads\n");
        exit(EXIT_FAILURE);
    }

    if ((code = pthread_mutex_init(&mutex_piece, NULL))) {
        my_perror("mutex_init", code);
        exit(EXIT_FAILURE);
    }
    if ((code = pthread_mutex_init(&mutex_result, NULL))) {
        my_perror("mutex_init", code);
        exit(EXIT_FAILURE);
    }
    pthread_t* threads = malloc(sizeof(pthread_t)*count_threads);
    if (threads == NULL) {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < count_threads; i++) {
        if ((code = pthread_create(&threads[i], NULL, &calc_series,  i)) != 0) {
            fprintf(stderr, "pthread_create error: %d \n", code);
            exit(EXIT_FAILURE);
        }
    }

    if (SIG_ERR == signal(SIGINT, handler)) {
        perror("signal failed");
        exit(EXIT_FAILURE);
    }

    double pi = 0;
    void* ret;
    for (int i = 0; i < count_threads; i++) {
        if ((code = pthread_join(threads[i], &ret)) != 0) {
            fprintf(stderr, "pthread_join error: %d\n", code);
            exit(EXIT_FAILURE);
        }

        if (ret == NULL) {
            fprintf(stderr, "An error occurred during the calculation of pi\n");
            exit(EXIT_FAILURE);
        }

        pi += *(double*)ret;
    }



    pi *= 4;
    printf("pi done - %.15g \n", pi);    

    exit(EXIT_SUCCESS);
}