CC = gcc
CFLAGS = -Wall -Wextra
LDFLAGS = -lsocket -lnsl

all: server client

server: server.c shared.h
	$(CC) $(CFLAGS) -o server server.c $(LDFLAGS)

client: client.c shared.h
	$(CC) $(CFLAGS) -o client client.c $(LDFLAGS)