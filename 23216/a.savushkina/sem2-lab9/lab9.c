#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <stdatomic.h>
#include <errno.h>

#define CHUNK_SIZE 1000000

#define handle_error_en(en, msg) \
    do { \
        errno = en; \
        perror(msg); \
        exit(EXIT_FAILURE); \
    } while (0)

atomic_int stop_flag = 0;
int num_threads;
unsigned long global_max_iter = 0;
pthread_mutex_t iter_mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
    long start;
    double partial_sum;
    unsigned long iterations;
} thread_data;

void sigint_handler() {
    atomic_store(&stop_flag, 1);
}

void* calculate_partial(void* arg) {
    thread_data* data = (thread_data*)arg;
    data->partial_sum = 0.0;
    data->iterations = 0;
    long i = data->start;
    int s;

    while (1) {
        double chunk_sum = 0.0;
        for (int k = 0; k < CHUNK_SIZE; k++) {
            chunk_sum += 1.0/(i*4.0 + 1.0);
            chunk_sum -= 1.0/(i*4.0 + 3.0);
            i += num_threads;
        }
        
        data->partial_sum += chunk_sum;
        data->iterations += CHUNK_SIZE;

        if ((s = pthread_mutex_lock(&iter_mutex))) 
            handle_error_en(s, "pthread_mutex_lock");

        if (data->iterations > global_max_iter) {
            global_max_iter = data->iterations;
        }
        if ((s = pthread_mutex_unlock(&iter_mutex))) 
            handle_error_en(s, "pthread_mutex_unlock");

        if (atomic_load(&stop_flag)) {
            if ((s = pthread_mutex_lock(&iter_mutex))) 
                handle_error_en(s, "pthread_mutex_lock");

            unsigned long target_iter = global_max_iter;

            if ((s = pthread_mutex_unlock(&iter_mutex))) 
                handle_error_en(s, "pthread_mutex_unlock");
            
            if (data->iterations >= target_iter) break;
        }
    }

    pthread_exit(data);
}

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s threads\n", argv[0]);
        return EXIT_FAILURE;
    }

    num_threads = atoi(argv[1]);
    if (num_threads <= 0) {
        fprintf(stderr, "Number of threads must be positive\n");
        exit(EXIT_FAILURE);
    }

    struct sigaction sa = {
        .sa_handler = sigint_handler,
        .sa_flags = 0
    };
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGINT, &sa, NULL) == -1){
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
        

    pthread_t threads[num_threads];
    thread_data* thread_args[num_threads];

    for (int t = 0; t < num_threads; t++) {
        thread_args[t] = malloc(sizeof(thread_data));
        if (thread_args[t] == NULL){
            perror("malloc");
            for (int i = 0; i < t; i++) {
                free(thread_args[i]);
            }
            exit(EXIT_FAILURE);
        }
            
        
        thread_args[t]->start = t;
        int s;
        if ((s = pthread_create(&threads[t], NULL, calculate_partial, thread_args[t])))
            handle_error_en(s, "pthread_create");
    }

    double total_sum = 0.0;
    for (int t = 0; t < num_threads; t++) {
        thread_data* result;
        int s;
        if ((s = pthread_join(threads[t], (void**)&result)))
            handle_error_en(s, "pthread_join");
        
        total_sum += result->partial_sum;
        free(result);
    }

    printf("\nPi approximation: %.15f\n", total_sum * 4.0);
    
    int s;
    if ((s = pthread_mutex_destroy(&iter_mutex)))
        handle_error_en(s, "pthread_mutex_destroy");
    
    return EXIT_SUCCESS;
}
// 3.14159265278502
// 3.1415926535 62232
// 3.1415926535 84949
// 3,1415926535 8979323846 2643383279 5028841971 
// 6939937510 5820974944 5923078164 0628620899 8628034825 
// 3421170679 8214808651 3282306647 0938446095 5058223172 
// 5359408128 4811174502 8410270193 8521105559 6446229489 
// 5493038196 4428810975 6659334461 2847564823 3786783165 2712019091