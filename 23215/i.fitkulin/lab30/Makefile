all: client.out server.out

client.out: client.c
	gcc -Wall client.c -o client.out -lnsl -lsocket 

server.out: server.c
	gcc -Wall server.c -o server.out -lnsl -lsocket
