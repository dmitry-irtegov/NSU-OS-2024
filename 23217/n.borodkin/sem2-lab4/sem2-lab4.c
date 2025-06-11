#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

void *print_str(){
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    while (1){
        printf("Дочерняя нить работает\n");
        sleep(1);
    }
    return NULL;
}
int main(){
    pthread_t thread;
    int res = pthread_create(&thread, NULL, print_str, NULL);
    if(res != 0){
        printf("Ошибка создания потока");
        return EXIT_FAILURE;
    }
    sleep(2);

    res = pthread_cancel(thread);
    if(res != 0){
        printf("Ошибка завершения потока");
        return EXIT_FAILURE;
    }

    printf("Программа завершена\n");
    res = pthread_join(thread, NULL);
    
    if(res != 0){
        printf("Ошибка ожидания завершения потока");
        return EXIT_FAILURE;
    }
}
