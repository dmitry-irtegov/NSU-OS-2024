all: lab3.out file

file:
	touch file
	chmod 600 file

lab3.out: lab3.c
	cc lab3.c -v -o lab3.out
	chmod u+s lab3.out
