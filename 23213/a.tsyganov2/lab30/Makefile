all: cli srv

cli: cli.c
	cc -v cli.c -o cli -lnsl -lsocket

srv: srv.c
	cc -v srv.c -o srv -lnsl -lsocket
