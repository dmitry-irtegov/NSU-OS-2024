all: compile_lab11 compile_test_prog

compile_lab11: lab11.c
	cc lab11.c -o lab11 -v 

compile_test_prog: print_environ.c
	cc print_environ.c -o print_environ -v


clean: 
	rm -f print_environ lab11
