CC = cc
CFLAGS = -v
LDFLAGS = -lsocket -lnsl

all: server client

server: server.c
	$(CC) $(CFLAGS) server.c $(LDFLAGS) -o server

client: client.c
	$(CC) $(CFLAGS) client.c $(LDFLAGS) -o client

