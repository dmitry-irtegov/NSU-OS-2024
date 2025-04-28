#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#define num_steps 200000000  

typedef struct {
    int start;     
    int end;       
    double sum;    
} ThreadData;

void *calculate_pi(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    double sum = 0.0;
    for (int i = data->start; i < data->end; i++) {
        sum += 1.0 / (i * 4.0 + 1.0);
        sum -= 1.0 / (i * 4.0 + 3.0);
    }
    data->sum = sum;
    pthread_exit(NULL);
}


int main(int argc, char **argv) {
    int num_threads = 1; 
    if (argc > 1) {  
        char *arg_ptr = argv[1], *endptr;
        int num = strtol(arg_ptr, &endptr, 10);
        if(*endptr != '\0' || num < 1) {
            fprintf(stderr, "incorrect number of threads\n");
            exit(1);
        }
        else {
            num_threads = num;
        }

    }

    pthread_t threads[num_threads];
    ThreadData thread_data[num_threads];
    int base_chunk = num_steps / num_threads;
    int ost = num_steps % num_threads;

    int current_start = 0;
    int code;
    char* buf;

    for (int i = 0; i < num_threads; i++) {
        int extra = (i < ost) ? 1 : 0; 
        thread_data[i].start = current_start;
        thread_data[i].end = current_start + base_chunk + extra;
        current_start = thread_data[i].end;
        code = pthread_create(&threads[i], NULL, calculate_pi, &thread_data[i]);
        if (code != 0) {
            buf = strerror(code);
            fprintf(stderr, "%s: creating thread %d: %s\n", argv[0], i, buf);
            exit(1);
        }
    }

    double pi = 0.0;
    for (int i = 0; i < num_threads; i++) {
        code = pthread_join(threads[i], NULL);
        if(code != 0){
            buf = strerror(code);
            fprintf(stderr, "%s: joining thread %d: %s\n", argv[0], i, buf);
            exit(1);
        }
        pi += thread_data[i].sum;
    }

    pi *= 4.0;
    printf("%.15g\n", pi);
    
    return 0;
}