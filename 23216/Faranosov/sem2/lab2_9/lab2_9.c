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
pthread_mutex_t mutex;
int isWork;


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
    int finish = start + 200;
    int needWork = 1;

    while (needWork) {

        for (int i = start; i < finish; i += data->step) {
            data->res += 1.0 / (i * 4.0 + 1.0);
            data->res -= 1.0 / (i * 4.0 + 3.0);
        }


        start = finish;
        finish = start + 200;

        res = pthread_mutex_lock(&mutex);
        if (res) handler("mutex lock error", res);

        needWork = isWork;

        res = pthread_mutex_unlock(&mutex);
        if (res) handler("mutex unlock error", res);
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


void sig_handler() {
    
    int res = 0;
    double pi = 0;

    res = pthread_mutex_lock(&mutex);
    if (res) handler("mutex lock error", res);

    isWork = 0;

    res = pthread_mutex_unlock(&mutex);
    if (res) handler("mutex unlock error", res);

    for (int i = 0; i < cntThr; i++) {
        res = pthread_join(threads[i], NULL);
        if (res) handler("join", res);
    }

    for (int i = 0; i < cntThr; i++) {
        
        pi += datas[i]->res;
        free(datas[i]);
    }
    free(threads);

    pi = pi * 4;
    printf("\npi done - %.15g \n", pi);

    res = pthread_attr_destroy(&attr);
    if (res) handler("attr destroy with error", res);

    res = pthread_mutex_destroy(&mutex);
    if (res) handler("mutex destroy with error", res);

    exit(EXIT_SUCCESS);
}


int main(int argc, char** argv) {
    isWork = 1;


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

    int check = 0;

    check = pthread_mutex_init(&mutex, NULL);
    if (check != 0) {
        handler("mutex init", check);
    }

    check = pthread_attr_init(&attr);
    if (check != 0) {
        handler("init", check);
    }

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

    for (int j = 0; j < cntThr; j++) {
        data* d = NULL;

        check = pthread_join(threads[j], (void**)&d);
        if (check != 0) {
            handler("join error", check);
        }
    }


    return (EXIT_SUCCESS);
}