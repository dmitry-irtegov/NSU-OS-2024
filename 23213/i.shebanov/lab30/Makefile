all: client.out server.out

client.out: client.c
	cc -v client.c -o client.out -lnsl -lsocket 

server.out: server.c
	cc -v server.c -o server.out -lnsl -lsocket
