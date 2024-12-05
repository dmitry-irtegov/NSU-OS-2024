all: server client

server: server.c 
		gcc -o server server.c -Wall -lsocket -lnsl
		
client: client.c
		gcc -o client client.c -Wall -lsocket -lnsl