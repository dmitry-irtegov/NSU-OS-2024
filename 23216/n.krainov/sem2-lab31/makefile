all: proxy
shell:
	gcc cache.o main.o proxy.o util.o statuses.o -Wall -Wextra -lsocket -lnsl -o proxy
cache.o: cache.c
	gcc cache.c -Wall -Wextra -o cache.o
main.o: main.o
	gcc main.c -Wall -Wextra -o 