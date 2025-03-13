#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define ITERATIONS 100000

typedef struct{
    int index;
    int step;
    double partiual_sum;
}ThreadData;

void* computing_pi(void* arg){
    ThreadData *data=(ThreadData*)arg;
    double sum = 0.0;
    for(int i = data->index;i<ITERATIONS;i+=data->step){
        sum += 1.0/(i*4.0 + 1.0);
        sum -= 1.0/(i*4.0 + 3.0);
    }
    data->partiual_sum=sum;
    pthread_exit(NULL);
}

void main(int argc, char *argv[]){
    int num_threads = atoi(argv[1]);
    pthread_t threads[num_threads];
    ThreadData data_threads[num_threads];
    for(int i = 0;i<num_threads;i++){
        data_threads[i].index=i;
        data_threads[i].step=num_threads;
        data_threads[i].partiual_sum=0.0;
        if(pthread_create(&threads[i], NULL, computing_pi, &data_threads[i])!=0){
            perror("pthread_create");
            exit(1);
        }
    }
    double final_pi=0.0;
    for(int i = 0;i<num_threads;i++) {
        pthread_join(threads[i], NULL);
        final_pi+=data_threads[i].partiual_sum;
    }
    final_pi*=4.0;
    printf("pi done - %.15g \n", final_pi);
    exit(0);
}