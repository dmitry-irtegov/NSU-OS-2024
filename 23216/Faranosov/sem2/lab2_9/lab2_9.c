#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <signal.h>


typedef struct data {
    int start_i;
    int step;
    double res;
} data;

int cntThr = 0;
pthread_t* threads;
data** datas;
pthread_attr_t attr;
pthread_mutex_t mainMutex, thrsMutex;
int isWork;
int maxIt;
int cntSleep;
pthread_cond_t mainCond;
pthread_cond_t thrsCond;


void handler(char str[], int num) {
    char buf[256];
    strerror_r(num, buf, 256);
    fprintf(stderr, "%s error: %s", str, buf);
    exit(EXIT_FAILURE);
}

void* func(void* param) {
    data* data = param;
    int res = 0;

#ifdef DEBUG
    printf("Thread start_i == %d, end_i == %d\n", data->start_i, data->end_i);
#endif
    int start = data->start_i;
    int finish = start + 100;
    int needWork = 1;

    while (needWork) {

        for (int i = start; i < finish; i += data->step) {
            data->res += 1.0 / (i * 4.0 + 1.0);
            data->res -= 1.0 / (i * 4.0 + 3.0);
        }


        start = finish;
        finish = start + 200;


        needWork = isWork;


    }

    res = pthread_mutex_lock(&thrsMutex);
    if (res) handler("mutex thrs lock error", res);



    res = pthread_mutex_lock(&mainMutex);
    if (res) handler("mutex main (thrs) lock error", res);

    cntSleep++;
    maxIt = start > maxIt ? start : maxIt;

    res = pthread_cond_signal(&mainCond);
    if (res) handler("cond mian signal error", res);

    res = pthread_mutex_unlock(&mainMutex);
    if (res) handler("mutex main (thrs) unlock error", res);




    res = pthread_cond_wait(&thrsCond, &thrsMutex);
    if (res) handler("cond thrs wait error", res);

    res = pthread_mutex_unlock(&thrsMutex);
    if (res) handler("mutex thrs lock error", res);

    for (int i = start; i < maxIt; i += data->step) {
        data->res += 1.0 / (i * 4.0 + 1.0);
        data->res -= 1.0 / (i * 4.0 + 3.0);
    }


    pthread_exit(NULL);
}

void free_after_bad_malloc(int j, pthread_t* t, pthread_attr_t *attr, data** datas) {
    int res = 0;
    for (int i = 0; i < j; i++) {
        res = pthread_cancel(t[i]);
        if (res != 0) {
            fprintf(stderr, "OK, i`m done\n");
            exit(EXIT_FAILURE);
        }
        free(datas[i]);
    }

    res = pthread_attr_destroy(attr);
    if (res) handler("attr destroy with error", res);
    exit(EXIT_FAILURE);
}


void destroy() {
    int res = 0;

    res = pthread_attr_destroy(&attr);
    if (res) handler("attr destroy with error", res);

    res = pthread_mutex_destroy(&mainMutex);
    if (res) handler("mutex main destroy with error", res);

    res = pthread_mutex_destroy(&thrsMutex);
    if (res) handler("mutex thrs destroy with error", res);

    res = pthread_cond_destroy(&mainCond);
    if (res) handler("cond main destroy with error", res);

    res = pthread_cond_destroy(&thrsCond);
    if (res) handler("cond threads destroy with error", res);
}

void sig_handler() {
   
    isWork = 0;
}

void init() {

    int check = 0;


    check = pthread_mutex_init(&mainMutex, NULL);
    if (check != 0) {
        handler("mutex main init", check);
    }

    check = pthread_mutex_init(&thrsMutex, NULL);
    if (check != 0) {
        handler("mutex thrs init", check);
    }

    check = pthread_cond_init(&mainCond, NULL);
    if (check != 0) {
        handler("cond main init", check);
    }

    check = pthread_cond_init(&thrsCond, NULL);
    if (check != 0) {
        handler("cond threads init", check);
    }

    check = pthread_attr_init(&attr);
    if (check != 0) {
        handler("init", check);
    }


}

int main(int argc, char** argv) {
    isWork = 1;
    maxIt = 0;
    cntSleep = 0;
    int check = 0;
    double pi = 0;


    if (sigset(SIGINT, sig_handler) == SIG_ERR) {
        perror("sigset error");
        exit(EXIT_FAILURE);
    }

    if (argc != 2) {
        fprintf(stderr, "wrong args");
        exit(EXIT_FAILURE);
    }

    
    if ((cntThr = atoi(argv[1])) <= 0) {
        fprintf(stderr, "wrong args");
        exit(EXIT_FAILURE);
    }

#ifdef DEBUG
    printf("cntThr == %d\n", cntThr);
#endif


    threads = NULL;
    threads = malloc(sizeof(pthread_t) * cntThr);
    if (threads == NULL) {
        fprintf(stderr, "threads malloc error");
        exit(EXIT_FAILURE);
    }

    datas = NULL;
    datas = malloc(sizeof(data*) * cntThr);
    if (datas == NULL) {
        free(threads);
        fprintf(stderr, "threads malloc error");
        exit(EXIT_FAILURE);
    }

    printf("Start calc\n");
    for (int j = 0; j < cntThr; j++) {
        data* d = NULL;
        d = malloc(sizeof(data));
        if (d == NULL) {
            free_after_bad_malloc(j, threads, &attr, datas);
        }

        d->start_i = j;
        d->step = cntThr;
        datas[j] = d;


        check = pthread_create(&threads[j], &attr, func, d);
        if (check != 0) {
            char buf[256];
            strerror_r(check, buf, 256);
            fprintf(stderr, "create error: %s", buf);
            free_after_bad_malloc(j, threads, &attr, datas);
        }

    }


    check = pthread_mutex_lock(&mainMutex);
    if (check) handler("mutex main lock error", check);

    while (cntSleep < cntThr) {

        check = pthread_cond_wait(&mainCond, &mainMutex);
        if (check) handler("cond main wait error", check);

    }

    check = pthread_mutex_unlock(&mainMutex);
    if (check) handler("mutex main unlock error", check);

    check = pthread_mutex_lock(&thrsMutex);
    if (check) handler("mutex thrs (main) lock error", check);

    check = pthread_cond_broadcast(&thrsCond);
    if (check) handler("broadcast error", check);

    check = pthread_mutex_unlock(&thrsMutex);
    if (check) handler("mutex thrs (main) unlock error", check);


    for (int i = 0; i < cntThr; i++) {
        check = pthread_join(threads[i], NULL);
        if (check) handler("join", check);
    }

    for (int i = 0; i < cntThr; i++) {

        pi += datas[i]->res;
        free(datas[i]);
    }
    free(threads);

    pi = pi * 4;
    printf("\npi done - %.15g \n", pi);

    destroy();

    exit(EXIT_SUCCESS);
}