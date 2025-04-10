CC = gcc
CFLAGS = -g -Wall -pthread -lrt

EXECS = drp dstr fstcns fstprd longmsg

all: $(EXECS)

drp: drp.c queue.c
	$(CC) $(CFLAGS) drp.c queue.c -o drp

dstr: dstr.c queue.c
	$(CC) $(CFLAGS) dstr.c queue.c -o dstr

fstcns: fstcns.c queue.c
	$(CC) $(CFLAGS) fstcns.c queue.c -o fstcns

fstprd: fstprd.c queue.c
	$(CC) $(CFLAGS) fstprd.c queue.c -o fstprd

longmsg: longmsg.c queue.c
	$(CC) $(CFLAGS) longmsg.c queue.c -o longmsg

clean:
	rm -f $(EXECS)
