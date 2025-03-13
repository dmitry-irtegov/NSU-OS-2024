#include <semaphore.h>
#include <pthread.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#define MSG_COUNT 10
#define BUFF_SIZE 80

typedef struct {
    char msgs[MSG_COUNT][BUFF_SIZE];
    sem_t avail_msg, avail_space;
    unsigned int next_free_index, next_msg_index;
    pthread_mutex_t lock;
    unsigned int curr_wait_msg, curr_wait_space;
    unsigned char is_dropped;
} Queue;

void mymsginit(Queue* queue) {
    sem_init(&queue->avail_msg, 0, 0);
    sem_init(&queue->avail_space, 0, 0);
    queue->curr_wait_space = queue->curr_wait_msg = queue->is_dropped = 0;
    queue->next_free_index= queue->next_msg_index = 0; 
    pthread_mutex_init(&queue->lock, NULL);
}

void mymsgdestroy(Queue* queue) {
    pthread_mutex_destroy(&queue->lock);
    sem_destroy(&queue->avail_msg);
    sem_destroy(&queue->avail_space);
}

int mymsgput(Queue* queue, char* msg) {
    pthread_mutex_lock(&queue->lock);
    if (queue->is_dropped) {
        return 0;
    }
    queue->curr_wait_space++;
    pthread_mutex_unlock(&queue->lock);
    
    sem_wait(&queue->avail_space);

    pthread_mutex_lock(&queue->lock);
    queue->curr_wait_space--;
    unsigned long msg_len = strlen(msg);
    unsigned long result_len = (BUFF_SIZE < msg_len) ? BUFF_SIZE : msg_len;
    int new_msg_index = queue->next_free_index;
    strncpy(queue->msgs[new_msg_index], msg, result_len);
    queue->next_free_index = (new_msg_index + 1) % MSG_COUNT;

    pthread_mutex_unlock(&queue->lock);
    
    sem_post(&queue->avail_msg);
    return result_len;
}

int mymsgget(Queue* queue, char* buf, size_t bufsize) {
    pthread_mutex_lock(&queue->lock);
    if (queue->is_dropped) {
        return 0;
    }
    queue->curr_wait_msg++;
    pthread_mutex_unlock(&queue->lock);

    sem_wait(&queue->avail_msg);
    
    pthread_mutex_lock(&queue->lock);
    
    queue->curr_wait_msg--;
    unsigned int msg_index = queue->next_msg_index;
    unsigned long msg_len = strlen(queue->msgs[msg_index]);
    unsigned long result_len = (bufsize < msg_len) ? bufsize : msg_len;
    strncpy(buf, queue->msgs[msg_index], result_len);
    queue->next_msg_index = (msg_index + 1) % MSG_COUNT;
    
    pthread_mutex_unlock(&queue->lock);

    sem_post(&queue->avail_space);
    return result_len;
}

void mymsgdrop(Queue* queue) {
    pthread_mutex_lock(&queue->lock);
    
    queue->is_dropped = 1;
    
    int wait_msg_count = queue->curr_wait_msg;
    for (int i = 0; i < wait_msg_count; i++) {
        sem_post(&queue->avail_msg);
    }
    int wait_space_count = queue->curr_wait_space;
    for (int i = 0; i < wait_space_count; i++) {
        sem_post(&queue->avail_space);
    }
    queue->curr_wait_space = queue->curr_wait_msg = 0;

    pthread_mutex_unlock(&queue->lock);
}

Queue queue;
int msg_received[2] = {0};

void* producer(void* arg) {
    char **msgs = (char**) arg;
    for (int i = 0; i < MSG_COUNT; i++) {
        assert(mymsgput(&queue, msgs[i]) > 0);   
    }
    return NULL;
}

void* consumer(void* arg) {
    unsigned long consumer_num = (unsigned long) arg;
    char buff[BUFF_SIZE + 1];
    int count_bytes = 0;
    do {
        count_bytes = mymsgget(&queue, buff, BUFF_SIZE);
        if (count_bytes > 0)
            msg_received[consumer_num]++;
        buff[BUFF_SIZE] = '\0';
        printf("%s\n", buff);
    } while (count_bytes > 0);
    return NULL;
}

int main() {
    mymsginit(&queue);
    char* msgs1[MSG_COUNT] = {
        "a",
        "aa",
        "aaa",
        "aaaa",
        "aaaaa",
        "aaaaaa",
        "aaaaaaa",
        "aaaaaaaa",
        "aaaaaaaaa",
        "aaaaaaaaaa"
    };
    char* msgs2[MSG_COUNT] = {
        "b",
        "bb",
        "bbb",
        "bbbb",
        "bbbbb",
        "bbbbbb",
        "bbbbbbb",
        "bbbbbbbb",
        "bbbbbbbbb",
        "bbbbbbbbbb"
    };
    
    pthread_t ths[4];
    pthread_create(&ths[0], NULL, consumer, 0);
    pthread_create(&ths[1], NULL, consumer, (void*) 1);

    pthread_create(&ths[0], NULL, producer, msgs1);
    pthread_create(&ths[1], NULL, producer, msgs2);

    for (int i = 0; i < 4; i++) {
        pthread_join(ths[i], NULL);
    }

    assert((2 * MSG_COUNT) == (msg_received[0] + msg_received[1]));

    mymsgdestroy(&queue);
    
    return 0;
}
