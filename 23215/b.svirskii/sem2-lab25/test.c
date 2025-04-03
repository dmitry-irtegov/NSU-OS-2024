#include <assert.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "queue.h"
#define CONSUMERS_COUNT 2
#define PRODUCERS_COUNT 2
#define THREADS_MSG_COUNT 20

void my_assert(int expr) {
    assert(expr);
}

Queue queue;
unsigned int bytes_received[CONSUMERS_COUNT] = {0};
unsigned int bytes_produced[PRODUCERS_COUNT] = {0};

char* msgs[PRODUCERS_COUNT][THREADS_MSG_COUNT] = {
    { 
        "Привет, мир!",
        "C — это мощно!",
        "Программирую на Си.",
        "Hello, World!",
        "Учим указатели...",
        "Безопасность важна.",
        "Компилятор GCC рулит.",
        "Отладка — это магия.",
        "Структуры данных — ключ.",
        "Алгоритмы на Си.",
        "Утечки памяти — зло.",
        "Функции и модульность.",
        "Работа с файлами.",
        "Динамическая память.",
        "Стек vs куча.",
        "Битовые операции.",
        "Сетевые возможности.",
        "Многопоточность в Си.",
        "Оптимизация кода.",
        "Си — это быстро!"
    }, {
        "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", 
            "13", "14", "15", "16", "17", "18", "19"
    }
};

void* producer(void* arg) {
    uint64_t producer_num = (uint64_t) arg;
    unsigned int bytes_putted;
    for (int i = 0; i < THREADS_MSG_COUNT; i++) {
        my_assert((bytes_putted = mymsgput(&queue, msgs[producer_num][i])) > 0);
        bytes_produced[producer_num] += bytes_putted;
        printf("producer %lu: produced \"%s\"\n", producer_num, msgs[producer_num][i]);
    }
    printf("%lu producer finished\n", producer_num);
    return NULL;
}

void* consumer(void* arg) {
    unsigned long consumer_num = (unsigned long) arg;
    char buff[BUFF_SIZE + 1];
    int count_bytes = 0;
    do {
        count_bytes = mymsgget(&queue, buff, BUFF_SIZE);
        bytes_received[consumer_num] += count_bytes;
        buff[count_bytes] = '\0';
        printf("consumer %lu: consumed \"%s\"\n", consumer_num, buff);
    } while (count_bytes > 0);
    printf("%lu consumer finished\n", consumer_num);
    return NULL;
}

int main() {
    mymsginit(&queue);
    
    pthread_t ths[PRODUCERS_COUNT + CONSUMERS_COUNT];

    for (uint64_t i = 0; i < PRODUCERS_COUNT; i++) {
        my_assert(0 == pthread_create(&ths[i], NULL, producer, (void*) i));
    }
    
    for (uint64_t i = 0; i < CONSUMERS_COUNT; i++) {
        my_assert(0 == pthread_create(&ths[PRODUCERS_COUNT + i], NULL, consumer, 
                    (void*) i));
    }

    sleep(1);
    
    mymsgdrop(&queue);
    
    for (int i = 0; i < PRODUCERS_COUNT + CONSUMERS_COUNT; i++) {
        pthread_join(ths[i], NULL);
    }
    
    unsigned long all_bytes_produced = 0;

    for (int i = 0; i < PRODUCERS_COUNT; i++) {
        all_bytes_produced += bytes_produced[i];
    }
    
    unsigned long all_bytes_consumed = 0;

    for (int i = 0; i < CONSUMERS_COUNT; i++) {
        all_bytes_consumed += bytes_received[i];
    }

//    assert(all_bytes_consumed == all_bytes_produced);
    
    mymsgdestroy(&queue);
    
    return 0;
}
