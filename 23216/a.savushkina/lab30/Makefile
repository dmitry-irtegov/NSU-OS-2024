CC = gcc
CFLAGS = -Wall -Wextra -lsocket -lnsl 

all: server client

server: lab30-server.c
	$(CC) $(CFLAGS) -o server lab30-server.c

client: lab30-client.c
	$(CC) $(CFLAGS) -o client lab30-client.c

clean:
	rm -f server client
