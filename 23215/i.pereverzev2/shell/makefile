FLAGS=-c -Wall -Wextra

all: shell

shell: shell.o parseline.o promptline.o jobutils.o
	gcc shell.o jobutils.o parseline.o promptline.o -o shell

shell.o: shell.c
	gcc $(FLAGS) shell.c

jobutils.o: jobutils.c
	gcc $(FLAGS) jobutils.c

parseline.o: parseline.c
	gcc $(FLAGS) parseline.c

promptline.o: promptline.c
	gcc $(FLAGS) promptline.c

clean:
	rm *.o
