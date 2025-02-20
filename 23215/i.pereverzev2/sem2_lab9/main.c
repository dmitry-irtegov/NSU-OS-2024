#define _REENTRANT
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <thread.h>
#include <unistd.h>
#include <string.h>

#define num_steps_check 20000

int intrpt;

typedef struct args_s {
    double partsum;
    int from;
    int everyn;
} args;

void handler(int signum) {
    intrpt = 1;
}

void* calc_partsum(void* param) {
    args* sumfrom = (args*)param;
    long long div = sumfrom->from;
    while(!intrpt) {
        for (int i = 0; i < num_steps_check; i++)
        {
            sumfrom->partsum += 1.0/(div*4.0 + 1.0);
            sumfrom->partsum -= 1.0/(div*4.0 + 3.0);
            div += sumfrom->everyn;
        }
    }
    
    pthread_exit(&(sumfrom->partsum));
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

    intrpt = 0;    
    signal(SIGINT, handler);

    pthread_t* threadbuf = (pthread_t*)malloc(sizeof(pthread_t) * threadnum);
    args* argbuf = (args*)malloc(sizeof(args) * threadnum);

    for(int i = 0; i < threadnum; i++) {
        argbuf[i].everyn = threadnum;
        argbuf[i].from = i;
        argbuf[i].partsum = 0;
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
        double *partsum = 0;
        int code = pthread_join(threadbuf[i], (void*)&partsum);
        if(code){
            char buf[256];
            strerror_r(code, buf, sizeof(buf));
            fprintf(stderr, "unable to join thread: %s", buf);
            return 4;
        }
        pi += *partsum;
    }
    free(threadbuf);
    free(argbuf);

    pi = pi * 4.0;
    printf("%.15g \n", pi);    
    return 0;
}

