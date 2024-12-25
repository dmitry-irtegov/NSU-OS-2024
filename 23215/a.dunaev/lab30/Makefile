CC = gcc
CFLAGS = -Wall -Wextra -O2

all: server client

server: server.c
	$(CC) $(CFLAGS) -o server server.c

client: client.c
	$(CC) $(CFLAGS) -o client client.c

clean:
	rm -f server client

.PHONY: all
