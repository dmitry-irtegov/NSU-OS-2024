all: proxy
proxy: main.o proxy.o cache.o statuses.o util.o
	gcc -o proxy main.o proxy.o cache.o statuses.o util.o -lsocket -lnsl
main.o: main.c
	gcc -c main.c
proxy.o: proxy.c
    gcc -c proxy.c
cache.o: cache.c
    gcc -c cache.c
statuses.o: statuses.c
    gcc -c statuses.c
util.o: util.c
    gcc -c util.c
clean:
    rm -f *.o proxy