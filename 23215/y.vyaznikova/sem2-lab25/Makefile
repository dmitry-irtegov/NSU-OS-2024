all: fast_producer_test fast_consumer_test message_length_test
fast_producer_test: fast_producer_test.c task25.c
	gcc -Wall -pthread -c task25.c
	gcc -Wall -pthread -c fast_producer_test.c
	gcc -Wall -pthread task25.o fast_producer_test.o -o fast_producer_test
fast_consumer_test: fast_consumer_test.c task25.c
	gcc -Wall -pthread -c task25.c
	gcc -Wall -pthread -c fast_consumer_test.c
	gcc -Wall -pthread task25.o fast_consumer_test.o -o fast_consumer_test
message_length_test: message_length_test.c task25.c
	gcc -Wall -pthread -c task25.c
	gcc -Wall -pthread -c message_length_test.c
	gcc -Wall -pthread task25.o message_length_test.o -o message_length_test
clean:
	rm -f *.o fast_producer_test fast_consumer_test message_length_test
