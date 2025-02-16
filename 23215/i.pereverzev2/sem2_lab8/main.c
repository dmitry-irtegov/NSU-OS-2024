#define _REENTRANT
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <thread.h>
#include <unistd.h>
#include <string.h>

#define num_steps 20000000

typedef struct args_s {
    double partsum;
    int from;
    int everyn;
} args;

void* calc_partsum(void* param) {
    args* sumfrom = (args*)param;
    for (int i = sumfrom->from; i < num_steps; i += sumfrom->everyn)
    {
        sumfrom->partsum += 1.0/(i*4.0 + 1.0);
        sumfrom->partsum -= 1.0/(i*4.0 + 3.0);
    }
    return NULL;
}


int main(int argc, char** argv) {
    
    if(argc != 2) {
        fprintf(stderr, "incorrect input, usage: %s numthreads\n", argv[0]);
        return 1;
    }
    int threadnum = atoi(argv[1]);
    if(threadnum <= 0) {
        fprintf(stderr, "incorrect numthreads value\n");
        return 2;
    }
    
    pthread_t* threadbuf = (pthread_t*)malloc(sizeof(pthread_t) * threadnum);
    args* argbuf = (args*)malloc(sizeof(args) * threadnum);

    for(int i = 0; i < threadnum; i++) {
        int code = pthread_create(threadbuf + i, NULL, calc_partsum, argbuf + i);
        if(code){
            char buf[256];
            strerror_r(code, buf, sizeof(buf));
            fprintf(stderr, "unable to create thread: %s", buf);
            return 3;
        }
    }

    double pi = 0;
    for(int i = 0; i < threadnum; i++) {
        int code = pthread_join(threadbuf[i], NULL);
        if(code){
            char buf[256];
            strerror_r(code, buf, sizeof(buf));
            fprintf(stderr, "unable to join thread: %s", buf);
            return 4;
        }
        pi += argbuf[i].partsum;
    }
    free(threadbuf);
    free(argbuf);

    pi = pi * 4.0;
    printf("pi done - %.15g \n", pi);    
    return 0;
}

