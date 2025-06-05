#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <string.h>

#define ITERATIONS_PER_FLAG_CHECK 1000000

int num_threads = 0;
volatile sig_atomic_t flag = 0;
pthread_mutex_t barrier_mutex;
pthread_cond_t barrier_cv;
int barrier_threads_arrived;
int barrier_current_generation;
int total_threads_for_manual_barrier;

typedef struct {
    int thread_id;
    double partial_sum;
    int iterations_done;
} thread_data_t;

void *calculate_partial_sum(void *arg) {
    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset, SIGINT);
    if (pthread_sigmask(SIG_BLOCK, &sigset, NULL) != 0) {
        perror("Thread: failed to set sigmask");
        return NULL;
    }

    thread_data_t *data = (thread_data_t *)arg;
    double local_pi = 0.0;
    long long current_term_idx = data->thread_id;
    long long local_iterations_done = 0;

    for (;; current_term_idx += num_threads) {
        local_pi += 1.0 / (current_term_idx * 4.0 + 1.0);
        local_pi -= 1.0 / (current_term_idx * 4.0 + 3.0);
        local_iterations_done++;

        if (local_iterations_done % ITERATIONS_PER_FLAG_CHECK == 0) {
            pthread_mutex_lock(&barrier_mutex);
            int gen = barrier_current_generation;
            barrier_threads_arrived++;
            if (barrier_threads_arrived == total_threads_for_manual_barrier) {
                barrier_threads_arrived = 0;
                barrier_current_generation++;
                pthread_cond_broadcast(&barrier_cv);
            } else {
                while (gen == barrier_current_generation) {
                    int wait_rc = pthread_cond_wait(&barrier_cv, &barrier_mutex);
                    if (wait_rc == EINTR) {
                        continue;
                    }
                    if (wait_rc != 0) {
                        fprintf(stderr, "Thread %d: pthread_cond_wait failed with non-EINTR error %d: %s. Exiting.\n",
                                data->thread_id, wait_rc, strerror(wait_rc));
                        pthread_mutex_unlock(&barrier_mutex);
                        data->partial_sum = local_pi;
                        data->iterations_done = local_iterations_done;
                        return arg; 
                    }
                }
            }
            pthread_mutex_unlock(&barrier_mutex);

            if (flag) {
                break;
            }
        }
    }

    data->iterations_done = local_iterations_done;
    printf("Thread %d finished, iterations = %lld\n", data->thread_id, local_iterations_done);
    data->partial_sum = local_pi;
    return arg;
}

void sigint_handler(int sig) {
    (void)sig;
    flag = 1;
}

int main(int argc, char **argv) {
    double total_pi = 0.0;
    long long total_iterations = 0;
    pthread_t *thread_handles;
    thread_data_t *thread_args_array;
    int barrier_initialized = 0;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <number_of_threads>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    num_threads = atoi(argv[1]);
    if (num_threads < 1) {
        fprintf(stderr, "Number of threads must be >= 1\n");
        exit(EXIT_FAILURE);
    }

    thread_handles = malloc(num_threads * sizeof(pthread_t));
    thread_args_array = malloc(num_threads * sizeof(thread_data_t));

    if (thread_handles == NULL || thread_args_array == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        free(thread_handles);
        free(thread_args_array);
        exit(EXIT_FAILURE);
    }

    total_threads_for_manual_barrier = num_threads;
    barrier_threads_arrived = 0;
    barrier_current_generation = 0;

    if (pthread_mutex_init(&barrier_mutex, NULL) != 0) {
        perror("Error initializing mutex");
        free(thread_handles);
        free(thread_args_array);
        exit(EXIT_FAILURE);
    }

    if (pthread_cond_init(&barrier_cv, NULL) != 0) {
        perror("Error initializing condition variable");
        pthread_mutex_destroy(&barrier_mutex); 
        free(thread_handles);
        free(thread_args_array);
        exit(EXIT_FAILURE);
    }
    barrier_initialized = 1;

    if (signal(SIGINT, sigint_handler) == SIG_ERR) {
        perror("Failed to set SIGINT handler");
        free(thread_handles);
        free(thread_args_array);
        exit(EXIT_FAILURE);
    }


    for (int i = 0; i < num_threads; i++) {
        thread_args_array[i].thread_id = i;
        thread_args_array[i].partial_sum = 0.0;
        thread_args_array[i].iterations_done = 0;
        int create_code = pthread_create(&thread_handles[i], NULL, calculate_partial_sum, (void *)&thread_args_array[i]);
        if (create_code != 0) {
            fprintf(stderr, "pthread_create error for thread %d: %s\n", i, strerror(create_code));
            flag = 1;
            for (int k = 0; k < i; k++) {
                pthread_join(thread_handles[k], NULL);
            }
            if (barrier_initialized) {
                pthread_mutex_destroy(&barrier_mutex);
                pthread_cond_destroy(&barrier_cv);
                barrier_initialized = 0;
            }
            free(thread_handles);
            free(thread_args_array);
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < num_threads; i++) {
        void *result_ptr;
        int join_code;
        while ((join_code = pthread_join(thread_handles[i], &result_ptr)) == EINTR) {
            continue;
        }
        if (join_code != 0) {
             fprintf(stderr, "pthread_join error for thread %d: %s. Results might be inaccurate.\n", i, strerror(join_code));
        } else {
            if (result_ptr != NULL) {
                total_pi += ((thread_data_t *)result_ptr)->partial_sum;
                total_iterations += ((thread_data_t *)result_ptr)->iterations_done;
            } else {
                 fprintf(stderr, "Warning: Thread %d returned NULL. Results might be inaccurate.\n", i);
            }
        }
    }
    total_pi *= 4.0;
    printf("Calculated pi = %.16f (using %lld iterations)\n", total_pi, total_iterations);
    free(thread_handles);
    free(thread_args_array);

    return EXIT_SUCCESS;
}