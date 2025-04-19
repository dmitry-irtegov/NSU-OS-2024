#include <stdlib.h>
#include <pthread.h>

#define QUEUE_CAPACITY 10
#define MSG_BUF_SIZE 81

typedef struct queue {
    pthread_cond_t cond;
    pthread_mutex_t mutex;
    size_t head;
    size_t len;
    char dropped;

    char msgs[QUEUE_CAPACITY][MSG_BUF_SIZE];
} queue;

void mymsginit(queue *);

void mymsgdrop(queue *);

void mymsgdestroy(queue *);

size_t mymsgput(queue *, char *msg);

size_t mymsgget(queue *, char *buf, size_t bufsize);
