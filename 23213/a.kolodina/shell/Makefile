shell.out: parseline.o promptline.o jobs.o shell.c shell.h
	cc -v parseline.o promptline.o jobs.o shell.c -o shell.out
parseline.o: parseline.c
	cc -c -v parseline.c -o parseline.o
promptline.o: promptline.c
	cc -c -v promptline.c -o promptline.o
jobs.o: jobs.c
	cc -c -v jobs.c -o jobs.o
