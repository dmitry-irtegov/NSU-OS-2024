FLAGS=-c -Wall -Wextra

all: shell

shell: shell.o parseline.o promptline.o 
	gcc shell.o parseline.o promptline.o -o shell

shell.o: shell.c
	gcc $(FLAGS) shell.c

parseline.o: parseline.c
	gcc $(FLAGS) parseline.c

promptline.o: promptline.c
	gcc $(FLAGS) promptline.c

clean:
	rm *.o
