#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#define num_steps 800000000
typedef struct params{
    int left;
    int right;
    double res;
}params;
void * calc_pi(void * arg){
    params * p = (params*) arg;
    double piPart = 0;

    for (int i = p->left; i < p->right ; i++) {
        piPart += 1.0/(i*4.0 + 1.0);
        piPart -= 1.0/(i*4.0 + 3.0);
    }
    p->res = piPart;
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

    params * threadsParams = (params*) calloc(threadAmount, sizeof(params));
    pthread_t * threads = (pthread_t*) malloc(sizeof(pthread_t) * threadAmount);
    
    int threadPart = (steps - 1) / threadAmount + 1;
    for(int i = 0; i < threadAmount; i++){
        params p = threadsParams[i];
        /*
        p->left = (int*)malloc(sizeof(int)); //вычисляем пи, начиная с элемента под таким номером
        params[1] = (int*)malloc(sizeof(int)); //и заканчиваем этим номером
        params[2] = (double*)malloc(sizeof(double)); //сюда положим результат*/
        p.left = i * threadPart;
        if(i == threadAmount - 1){
            p.right = steps;
        }
        else{
            p.right = (i+1) * threadPart;
        }
        threadsParams[i] = p;
        pthread_create(threads + i, NULL, calc_pi, threadsParams+i);
    }

    for(int i = 0; i < threadAmount; i++){
        pthread_join(threads[i], NULL);
        pi += threadsParams[i].res;
    }

    pi = pi * 4.0;
    printf("pi done - %.15g \n", pi);    
    
    free(threadsParams);
    free(threads);
    return (EXIT_SUCCESS);
}

