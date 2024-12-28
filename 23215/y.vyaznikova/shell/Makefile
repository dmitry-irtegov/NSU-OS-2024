.PHONY: all clean

CC = gcc
CFLAGS = -Wall -Wextra
DEPS = shell.h jobs.h command.h builtins.h
OBJ = parseline.o promptline.o jobs.o builtins.o

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

shell: $(OBJ) shell.c
	$(CC) $(CFLAGS) $(OBJ) shell.c -o shell

clean:
	rm -f *.o shell
