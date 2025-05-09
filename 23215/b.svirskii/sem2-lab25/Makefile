all: test slow_prod_test slow_cons_test

test: test.c
	gcc -Wall -pthread queue.c test.c -o test
slow_prod_test: slow_prod_test.c
	gcc -Wall -pthread queue.c slow_prod_test.c -o slow_prod_test
slow_cons_test: slow_cons_test.c
	gcc -Wall -pthread queue.c slow_cons_test.c -o slow_cons_test

clean:
	rm test slow_cons_test slow_prod_test
