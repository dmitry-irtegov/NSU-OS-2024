#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "queue.h"

void mymsginit(queue *q) {
	q->head=NULL;
	q->tail=NULL;
	q->beingdestroyed=0;
	sem_init(&(q->headsem), 0, 10);
	sem_init(&(q->tailsem), 0, 0);
	sem_init(&(q->queuesem), 0, 1);
}

void mymsgdrop(queue *q) {
	struct queue_record *t;
	q->beingdestroyed=1;
	sem_wait(&q->queuesem);
	sem_post(&q->headsem);
	sem_post(&q->tailsem);
	t=q->head;
	while(t) {
		struct queue_record *t1;
		t1=t->next;
		free(t);
		t=t1;
	}
	sem_post(&q->queuesem);
}

void mymsgdestroy(queue *q) {
	sem_destroy(&q->headsem);
	sem_destroy(&q->tailsem);
	sem_destroy(&q->queuesem);
}

int mymsgput(queue *q, char * msg) {
	struct queue_record *t;
		
	sem_wait(&q->headsem);
	sem_wait(&q->queuesem);
	if (q->beingdestroyed) {
		sem_post(&q->headsem);
		sem_post(&q->queuesem);
		return 0;
	}
	t=malloc(sizeof(struct queue_record));
	strncpy(t->buf, msg, sizeof(t->buf)-1);
	t->buf[sizeof(t->buf)-1]='\0';
	t->prev=NULL;
	t->next=q->head;
	if (q->head==NULL) {
            q->head=q->tail=t;
	} else {
            q->head->prev=t;
            q->head=t;
	}
	sem_post(&q->queuesem);
	sem_post(&q->tailsem);
	return strlen(t->buf)+1;
}

int mymsgget(queue *q, char *buf, size_t bufsize) {
	struct queue_record *t;

	sem_wait(&q->tailsem);
	sem_wait(&q->queuesem);
	if (q->beingdestroyed) {
		sem_post(&q->tailsem);
		sem_post(&q->queuesem);
		return 0;
	}
	t=q->tail;
	if (q->head==t) {
		q->head=q->tail=NULL;
	} else {
		q->tail=t->prev;
		q->tail->next=NULL;
	}
	sem_post(&q->queuesem);
	strncpy(buf, t->buf, bufsize-1);
	buf[bufsize-1]='\0';
	free(t);
	sem_post(&q->headsem);
	return strlen(buf)+1;
}


