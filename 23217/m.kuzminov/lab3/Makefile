all: create_execute_file create_file set_permissions

create_execute_file: prog.c
	gcc prog.c -o prog.out

create_file: 
	touch file
 
set_permissions:
	chmod 600 file
	chmod u+s prog.out

clean:
	rm -f file prog.out