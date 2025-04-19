#include "mymsg.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>

void mymsginit(queue *q) {
    int err = pthread_cond_init(&q->cond, NULL);
    if (err != 0) {
        fprintf(stderr, "mymsginit: Error initializing condition variable: %s\n", strerror(err));
        exit(1);
    }
    err = pthread_mutex_init(&q->mutex, NULL);
    if (err != 0) {
        fprintf(stderr, "mymsginit: Error initializing mutex: %s\n", strerror(err));
        exit(1);
    }

    q->head = 0;
    q->len = 0;
    q->dropped = 0;
}

void mymsgdrop(queue *q) {
    int err = pthread_mutex_lock(&q->mutex);
    if (err != 0) {
        fprintf(stderr, "mymsgdrop: Error locking mutex: %s\n", strerror(err));
        abort();
    }

    q->dropped = 1;
    
    err = pthread_mutex_unlock(&q->mutex);
    if (err != 0) {
        fprintf(stderr, "mymsgdrop: Error unlocking mutex: %s\n", strerror(err));
        abort();
    }
    
    err = pthread_cond_broadcast(&q->cond);
    if (err != 0) {
        fprintf(stderr, "mymsgdrop: Error broadcasting condition variable: %s\n", strerror(err));
        abort();
    }
}

void mymsgdestroy(queue *q) {
    int err = pthread_cond_destroy(&q->cond);
    if (err != 0) {
        fprintf(stderr, "mymsgdestroy: Error destroying condition variable: %s\n", strerror(err));
        abort();
    }
    err = pthread_mutex_destroy(&q->mutex);
    if (err != 0) {
        fprintf(stderr, "mymsgdestroy: Error destroying mutex: %s\n", strerror(err));
        abort();
    }
}

size_t mymsgput(queue *q, char *msg) {
    int err = pthread_mutex_lock(&q->mutex);
    if (err != 0) {
        fprintf(stderr, "mymsgput: Error locking mutex: %s\n", strerror(err));
        abort();
    }

    while (q->len == QUEUE_CAPACITY && !q->dropped) {
        err = pthread_cond_wait(&q->cond, &q->mutex);
        if (err != 0) {
            fprintf(stderr, "mymsgput: Error waiting on condition variable: %s\n", strerror(err));
            abort();
        }
    }

    if (q->dropped) {
        err = pthread_mutex_unlock(&q->mutex);
        if (err != 0) {
            fprintf(stderr, "mymsgput: Error unlocking mutex: %s\n", strerror(err));
            abort();
        }
        return 0;
    }

    int i = (q->head + q->len) % QUEUE_CAPACITY;

    size_t msglen = strnlen(msg, MSG_BUF_SIZE - 1);
    memcpy(q->msgs[i], msg, msglen);
    q->msgs[i][msglen] = '\0';

    q->len++;
    err = pthread_mutex_unlock(&q->mutex);
    if (err != 0) {
        fprintf(stderr, "mymsgput: Error unlocking mutex: %s\n", strerror(err));
        abort();
    }

    err = pthread_cond_signal(&q->cond);
    if (err != 0) {
        fprintf(stderr, "mymsgput: Error signaling condition variable: %s\n", strerror(err));
        abort();
    }

    return msglen + 1;
}

size_t mymsgget(queue *q, char *buf, size_t bufsize) {
    int err = pthread_mutex_lock(&q->mutex);
    if (err != 0) {
        fprintf(stderr, "mymsgget: Error locking mutex: %s\n", strerror(err));
        abort();
    }

    while (q->len == 0 && !q->dropped) {
        err = pthread_cond_wait(&q->cond, &q->mutex);
        if (err != 0) {
            fprintf(stderr, "mymsgget: Error waiting on condition variable: %s\n", strerror(err));
            abort();
        }
    }

    if (q->dropped) {
        err = pthread_mutex_unlock(&q->mutex);
        if (err != 0) {
            fprintf(stderr, "mymsgget: Error unlocking mutex: %s\n", strerror(err));
            abort();
        }
        return 0;
    }

    size_t msglen;
    if (bufsize > 0) {
        msglen = strlen(q->msgs[q->head]);
        if (msglen > bufsize) {
            msglen = bufsize - 1;
        }
        memcpy(buf, q->msgs[q->head], msglen);
        buf[msglen] = '\0';

        msglen++;
    } else {
        msglen = 0;
    }

    q->head = (q->head + 1) % QUEUE_CAPACITY;
    q->len--;
    err = pthread_mutex_unlock(&q->mutex);
    if (err != 0) {
        fprintf(stderr, "mymsgget: Error unlocking mutex: %s\n", strerror(err));
        abort();
    }

    err = pthread_cond_signal(&q->cond);
    if (err != 0) {
        fprintf(stderr, "mymsgget: Error signaling condition variable: %s\n", strerror(err));
        abort();
    }

    return msglen;
}
