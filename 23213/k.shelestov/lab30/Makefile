all: client.out server.out

client.out: client.c
	cc -v client.c -lnsl -lsocket -o client.out

server.out: server.c
	cc -v server.c -lnsl -lsocket -o server.out
