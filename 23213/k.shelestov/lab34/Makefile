run: recv.out trans.out client.out

recv.out: recv.o 
	cc -o recv.out recv.o  -lsocket -lnsl

trans.out: trans.o 
	cc -o trans.out trans.o  -lsocket -lnsl

client.out: client.o 
	cc -o client.out client.o  -lsocket -lnsl

recv.o: recv.c
	cc -c recv.c 

trans.o: trans.c
	cc -c trans.c

client.o: client.c
	cc -c client.c

clean:
	rm -rf *.o recv.out trans.out client.out

