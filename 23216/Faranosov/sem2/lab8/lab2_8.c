#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#define num_steps 200000000

typedef struct data {
    int start_i;
    int end_i;
    double res;
} data;


void handler(char str[], int num) {
    char buf[256];
    strerror_r(num, buf, 256);
    fprintf(stderr, "%s error: %s", str, buf);
    exit(EXIT_FAILURE);
}

void* func(void* param) {
    data* data = param;

#ifdef DEBUG
    printf("Thread start_i == %d, end_i == %d\n", data->start_i, data->end_i);
#endif

    double res = 0;
    for (int i = data->start_i; i < data->end_i; i++) {
        res += 1.0 / (i * 4.0 + 1.0);
        res -= 1.0 / (i * 4.0 + 3.0);
    }

#ifdef DEBUG
    printf("Thread res = %llf\n", res);
#endif // DEBUG

    data->res = res;
    pthread_exit(data);
}

void free_after_bad_malloc(int j, pthread_t* t, pthread_attr_t* attr, data** datas) {
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

int main(int argc, char** argv) {

    if (argc != 2) {
        fprintf(stderr, "wrong args");
        exit(EXIT_FAILURE);
    }

    int cntThr = 0;
    if ((cntThr = atoi(argv[1])) <= 0) {
        fprintf(stderr, "wrong args");
        exit(EXIT_FAILURE);
    }

#ifdef DEBUG
    printf("cntThr == %d\n", cntThr);
#endif

    double pi = 0;
    int check = 0;
    pthread_attr_t attr;

    check = pthread_attr_init(&attr);
    if (check != 0) {
        handler("init", check);
    }

    pthread_t* threads = NULL;
    threads = malloc(sizeof(pthread_t) * cntThr);
        if (threads == NULL) {
            fprintf(stderr, "threads malloc error");
            exit(EXIT_FAILURE);
        }

    data** datas = NULL;
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

        d->start_i = num_steps/cntThr * j;
        if (j == cntThr - 1) d->end_i = num_steps;
        else d->end_i = num_steps / cntThr * (j + 1);
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

        if (d != NULL) {

            pi += d->res;
            free(d);
        }
        else {
            fprintf(stderr, "return error");
            exit(EXIT_FAILURE);
        }
    }

    free(threads);
    free(datas);

    check = pthread_attr_destroy(&attr);
    if (check != 0) handler("attr destroy", check);


    pi = pi * 4.0;
    printf("pi done - %.15g \n", pi);

    return (EXIT_SUCCESS);
}
