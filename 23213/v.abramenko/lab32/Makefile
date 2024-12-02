all: client server

client: client.c
	cc -v client.c -o client -lnsl -lsocket 

server: server.c
	cc -v server.c -o server -laio -lnsl -lsocket
