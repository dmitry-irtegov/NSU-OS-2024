all: server client

.PHONY: all

server: server.c
	gcc server.c -o server -lnsl -lsocket -Wall -Werror -pedantic -std=c99

client: client.c
	gcc client.c -o client -lnsl -lsocket -Wall -Werror -pedantic -std=c99
