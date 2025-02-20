#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

typedef struct {
    long long index;
    double partsum;
} helper;

#define num_steps 2000000000

long long countThreads;

void* calculator(void* param) {
    double pi = 0.0;

    for (long long i = ((helper*)param)->index; i < num_steps; i += countThreads) {

        pi += 1.0 / (i * 4.0 + 1.0);
        pi -= 1.0 / (i * 4.0 + 3.0);
    }
    ((helper*) param)->partsum = pi;
    return param;
}

    int main(int argc, char** argv) {

        double globalpi = 0;

        pthread_t* idthreads;
        helper* params;

        if (argc >= 2) {
            countThreads = atol(argv[1]);
        } else {
            fprintf(stderr, "Not enough arguments");
            exit(EXIT_FAILURE);
        }

        idthreads = malloc(countThreads * sizeof(pthread_t));
        params = malloc (countThreads * sizeof(helper));

        for (int i = 0; i < countThreads; i++) {
            params[i].index = i;
            pthread_create(&(idthreads[i]), NULL, calculator, (void*)(params+i));
        }
        for (int i = 0; i < countThreads; i++) {
            helper* res;
            pthread_join(idthreads[i], (void**)&res);
            globalpi += res->partsum;
        }

        globalpi *= 4.0;

        printf("pi = %.15g\n", globalpi);
        
        free(idthreads);
        free(params);

        return(EXIT_SUCCESS);
        // как считают пи до тысяч знаков когда не хватает дабла

    }