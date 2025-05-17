#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <alloca.h>
#include <unistd.h>
#include <semaphore.h>
#include "queue.h"

void *producer(void *pq) {
	queue *q=(queue*)pq;
	int i=0;

	for(i=0; i<1000; i++) {
		char buf[40];
		sprintf(buf, "Packet %d [sender:%lu]", i, pthread_self());
		if (!mymsgput(q, buf)) return NULL;
	}

	return NULL;
}

void *consumer(void *pq) {
	queue *q=(queue *)pq;
	int i=0;

	do {
		char buf[41];
		i=mymsgget(q, buf, sizeof(buf));
		if (i==0) return NULL;
		else printf("Thread %lu received: %s\n", pthread_self(), buf);
	} while(1);
}

int main(int argc, char **argv) {
	int nproducers, nconsumers;
	queue q;
        int i;

	if (argc<3) {
		fprintf(stderr, "Usage: %s nproducers nconsumers\n", argv[0]);
		return 0;
	}

	nproducers=atol(argv[1]);
	nconsumers=atol(argv[2]);

	if (nproducers==0 || nconsumers==0) {
		fprintf(stderr, "Usage: %s nproducers nconsumers\n", argv[0]);
		return 0;
	}

	pthread_t producers[nproducers];
	pthread_t consumers[nconsumers];

	mymsginit(&q);

	for(i=0; i<nproducers || i<nconsumers; i++) {
		if (i<nproducers) {
			pthread_create(&producers[i], NULL, producer, &q);
		}
		if (i<nconsumers) {
			pthread_create(&consumers[i], NULL, consumer, &q);
		}
	}

	sleep(10);

	mymsgdrop(&q);
	for(i=0; i<nproducers || i<nconsumers; i++) {
		if(i<nproducers) {
			pthread_join(producers[i], NULL);
		}
		if (i<nconsumers) {
			pthread_join(consumers[i], NULL);
		}
	}

	mymsgdestroy(&q);
	printf("All threads quit and queue destroyed\n");
	return 0;
}
