all: flexible_test

test_suite: fast_producer_test fast_consumer_test message_length_test \
     multiple_fast_producers_test multiple_fast_consumers_test multiple_message_length_test
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
multiple_producers_consumers_test: multiple_producers_consumers_test.c task25.c
	gcc -Wall -pthread -c task25.c
	gcc -Wall -pthread -c multiple_producers_consumers_test.c
	gcc -Wall -pthread task25.o multiple_producers_consumers_test.o -o multiple_producers_consumers_test

multiple_fast_producers_test: multiple_fast_producers_test.c task25.c
	gcc -Wall -pthread -c task25.c
	gcc -Wall -pthread -c multiple_fast_producers_test.c
	gcc -Wall -pthread task25.o multiple_fast_producers_test.o -o multiple_fast_producers_test

multiple_fast_consumers_test: multiple_fast_consumers_test.c task25.c
	gcc -Wall -pthread -c task25.c
	gcc -Wall -pthread -c multiple_fast_consumers_test.c
	gcc -Wall -pthread task25.o multiple_fast_consumers_test.o -o multiple_fast_consumers_test

multiple_message_length_test: multiple_message_length_test.c task25.c
	gcc -Wall -pthread -c task25.c
	gcc -Wall -pthread -c multiple_message_length_test.c
	gcc -Wall -pthread task25.o multiple_message_length_test.o -o multiple_message_length_test

flexible_test: flexible_test.c task25.c
	gcc -Wall -pthread -c task25.c
	gcc -Wall -pthread -c flexible_test.c
	gcc -Wall -pthread task25.o flexible_test.o -o flexible_test

clean:
	rm -f *.o flexible_test fast_producer_test fast_consumer_test message_length_test \
        multiple_fast_producers_test multiple_fast_consumers_test multiple_message_length_test
