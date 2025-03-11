#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>

#define num_steps 2000000ull

typedef struct {
    pthread_t thread;
    int id;
    double result;
    char completed;
    char processed;
} thread_data;

thread_data* threads;

pthread_mutex_t mutex_piece, mutex;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
unsigned long long currentStart = 0;
int num_result;
char flag = 0;

void my_perror(char* text, int code) {
    fprintf(stderr, "%s: %s \n", text, strerror(code));
}

void handler() {
    int code;
    char* string = "pthread_mutex_lock failed in handler\n";
    if ((code = pthread_mutex_lock(&mutex_piece))) {
        write(STDERR_FILENO, string, strlen(string));
        _exit(EXIT_FAILURE);
    }
    flag = 1;
    if ((code = pthread_mutex_unlock(&mutex_piece))) {
        write(STDERR_FILENO, string, strlen(string));
        _exit(EXIT_FAILURE);
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

void* calc_series(void* data) {
    thread_data* thdata = (thread_data*) data;

    int code = 0;
    unsigned long long start, end;
    double res = 0;
    while (1) {
        if (getNextPiece(&start, &end) == 1) {
            if ((code = pthread_mutex_lock(&mutex))) {
                my_perror("pthread_mutex_lock", code);
                exit(EXIT_FAILURE);
            }
            thdata->completed = 1;
            thdata->result = res;
            if ((code = pthread_cond_signal(&cond))) {
                my_perror("pthread_cond_signal", code);
                exit(EXIT_FAILURE);
            }

            if ((code = pthread_mutex_unlock(&mutex))) {
                my_perror("pthread_mutex_unlock", code);
                exit(EXIT_FAILURE);
            }
            break;
        }

        for (unsigned long long i = start; i < end; i++) {    
            res += 1.0/(i*4.0 + 1.0);
            res -= 1.0/(i*4.0 + 3.0);
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
    if ((code = pthread_mutex_init(&mutex, NULL))) {
        my_perror("mutex_init", code);
        exit(EXIT_FAILURE);
    }

    thread_data* threads = calloc(count_threads, sizeof(thread_data));
    if (threads == NULL) {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < count_threads; i++) {
        threads[i].id = i;
        if ((code = pthread_create(&threads[i].thread, NULL, &calc_series, (void*)&threads[i])) != 0) {
            fprintf(stderr, "pthread_create error: %d \n", code);
            exit(EXIT_FAILURE);
        }
    }

    if (SIG_ERR == signal(SIGINT, handler)) {
        perror("signal failed");
        exit(EXIT_FAILURE);
    }

    double pi = 0;

    int processed_count = 0;
    while (processed_count < count_threads) {
        pthread_mutex_lock(&mutex);
        
        char found = 0;
        for (int i = 0; i < count_threads; i++) {
            if (threads[i].completed && !threads[i].processed) {
                threads[i].processed = 1;
                processed_count++;
                found = 1;
                pi += threads[i].result;
            }
        }
        
        if (!found) {
            if ((code = pthread_cond_wait(&cond, &mutex))) {
                my_perror("pthread_cond_wait", code);
                exit(EXIT_FAILURE);
            }
        }
        
        if ((code = pthread_mutex_unlock(&mutex))) {
            my_perror("pthread_mutex_unlock", code);
            exit(EXIT_FAILURE);
        }
    }

    pi *= 4;
    printf("pi done - %.15g \n", pi);    

    exit(EXIT_SUCCESS);
}