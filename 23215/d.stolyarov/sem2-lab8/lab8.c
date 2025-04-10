#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#define num_steps 800000000
void * calc_pi(void * params){
    double piPart = 0;
    int from = *((int*) ((void**)params)[0]);
    int to = *((int*) ((void**)params)[1]);

    for (int i = from; i < to ; i++) {
        piPart += 1.0/(i*4.0 + 1.0);
        piPart -= 1.0/(i*4.0 + 3.0);
    }
    *((double*)((void**)params)[2]) = piPart;
    pthread_exit(0);
}

int main(int argc, char** argv) {
    
    if(argc < 2){
        perror("Not enough params");
        exit(1);
    }
    double pi = 0;
    
    int steps;
    if(argc >= 3){
        steps = atoi(argv[2]);
    }
    else{
        steps = num_steps;
    }
    int threadAmount = atoi(argv[1]);
    if(threadAmount < 1){
        perror("Wrong amount of threads");
        exit(2);
    }

    void *** threadsParams = (void***) malloc(sizeof(void**) *  threadAmount);
    pthread_t * threads = (pthread_t*) malloc(sizeof(pthread_t) * threadAmount);
    
    int threadPart = (steps - 1) / threadAmount + 1;
    for(int i = 0; i < threadAmount; i++){
        void ** params = (void **)malloc(sizeof(void *) * 3);
        params[0] = (int*)malloc(sizeof(int)); //вычисляем пи, начиная с элемента под таким номером
        params[1] = (int*)malloc(sizeof(int)); //и заканчиваем этим номером
        params[2] = (double*)malloc(sizeof(double)); //сюда положим результат
        *((int*)params[0]) = i * threadPart;
        if(i == threadAmount - 1){
            *((int*)params[1]) = steps;
        }
        else{
            *((int*)params[1]) = (i+1) * threadPart;
        }
        pthread_create(threads + i, NULL, calc_pi, params);
        threadsParams[i] = params;
    }

    for(int i = 0; i < threadAmount; i++){
        pthread_join(threads[i], NULL);
        pi += *((double*)threadsParams[i][2]);
        free(threadsParams[i][0]);
        free(threadsParams[i][1]);
        free(threadsParams[i][2]);
        free(threadsParams[i]);
    }

    pi = pi * 4.0;
    printf("pi done - %.15g \n", pi);    
    
    free(threadsParams);
    free(threads);
    return (EXIT_SUCCESS);
}

