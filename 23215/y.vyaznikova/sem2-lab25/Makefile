TARGETS = task25_sem.o fast_producers_test fast_consumers_test flexible_test message_length_test
CFLAGS = -Wall -pthread

.PHONY: all clean

all: $(TARGETS)

clean:
	rm -f $(TARGETS) *.o

task25_sem.o: task25_sem.c task25_sem.h
	gcc $(CFLAGS) -c -o task25_sem.o task25_sem.c

fast_producers_test: fast_producers_test.c task25_sem.o
	gcc $(CFLAGS) -DUSE_SEMAPHORES -o fast_producers_test fast_producers_test.c task25_sem.o

fast_consumers_test: fast_consumers_test.c task25_sem.o
	gcc $(CFLAGS) -DUSE_SEMAPHORES -o fast_consumers_test fast_consumers_test.c task25_sem.o

flexible_test: flexible_test.c task25_sem.o
	gcc $(CFLAGS) -DUSE_SEMAPHORES -o flexible_test flexible_test.c task25_sem.o

message_length_test: message_length_test.c task25_sem.o
	gcc $(CFLAGS) -DUSE_SEMAPHORES -o message_length_test message_length_test.c task25_sem.o
