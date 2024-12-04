all: compile_server compile_client

compile_server: server.c
	cc server.c -o srv -v -lsocket

compile_client: client.c
	cc client.c -o cli -v -lsocket


clean: 
	rm -f srv cli
