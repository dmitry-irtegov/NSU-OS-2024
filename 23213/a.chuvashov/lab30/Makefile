all: server.out client.out

server.out: server.c
	cc -v server.c -lsocket -lnsl -o server.out

client.out: client.c
	cc -v client.c -lsocket -lnsl -o client.out
