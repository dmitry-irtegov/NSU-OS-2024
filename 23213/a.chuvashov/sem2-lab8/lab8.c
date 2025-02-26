#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

typedef struct Container {
    int index;
    double value;
} Container;

#define NUM_STEPS 200000000

int amount_of_threads;

void* pi_calculus(void* data) {
    double partial_pi;
    Container* values = (Container*)data;
    for (int i = values->index; i < NUM_STEPS ; i += amount_of_threads) {        
        partial_pi += 1.0/(i*4.0 + 1.0);
        partial_pi -= 1.0/(i*4.0 + 3.0);
    }
    values->value = partial_pi;
    return data;
}

int main(int argc, char** argv) {   
    if (argc != 2) {
        fprintf(stderr, "Wrong amount of arguments\n");
        return -1;
    }
    
    if ((amount_of_threads = atoi(argv[1])) < 1) {
        fprintf(stderr, "Number of threads should be an integer bigger than 0\n");
        exit(-1);
    }

    pthread_t* threads = (pthread_t*)malloc(amount_of_threads * sizeof(pthread_t));
    Container* containers = (Container*)malloc(amount_of_threads * sizeof(Container));

    for (int i = 0; i < amount_of_threads; i++) {
        int code;
        containers[i].index = i;
        containers[i].value = 0;
        if ((code = pthread_create(&threads[i], NULL, pi_calculus, (void*)&containers[i])) != 0) {
            fprintf(stderr, "%s: creating thread: %s\n", argv[0], strerror(code));
            exit(1);
        }
    }
    

    double pi = 0;
    for (int i = 0; i < amount_of_threads; i++) {
        pthread_join(threads[i], NULL);
        pi += containers[i].value;
    }
    
    pi *= 4.0;
    printf("pi done - %.15g \n", pi);    
    free(containers);
    free(threads);    
    return EXIT_SUCCESS;
}
