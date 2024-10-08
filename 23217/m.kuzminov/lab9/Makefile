all: createFile lab9.out


createFile:
	touch test.txt
	echo "TEXT FOR FILE text.txt :)" > test.txt

lab9.out: lab9.c
	cc lab9.c -o lab9.out -v

clean:
	rm lab9.out test.txt -f