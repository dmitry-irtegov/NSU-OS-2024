TARGETS = task25_cond.o fast_producers_test fast_consumers_test flexible_test message_length_test
CFLAGS = -Wall -pthread

.PHONY: all clean

all: $(TARGETS)

clean:
	rm -f $(TARGETS) *.o

task25_cond.o: task25_cond.c task25_cond.h
	gcc $(CFLAGS) -c -o task25_cond.o task25_cond.c

fast_producers_test: fast_producers_test.c task25_cond.o
	gcc $(CFLAGS) -o fast_producers_test fast_producers_test.c task25_cond.o

fast_consumers_test: fast_consumers_test.c task25_cond.o
	gcc $(CFLAGS) -o fast_consumers_test fast_consumers_test.c task25_cond.o

flexible_test: flexible_test.c task25_cond.o
	gcc $(CFLAGS) -o flexible_test flexible_test.c task25_cond.o

message_length_test: message_length_test.c task25_cond.o
	gcc $(CFLAGS) -o message_length_test message_length_test.c task25_cond.o
