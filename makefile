
shell: shell.o list.o listproc.o
	gcc -o shell shell.o list.o listproc.o

shell.o: shell.c list.h listproc.h
	gcc -c shell.c list.h listproc.h

list.o: list.c list.h
	gcc -c list.c list.h

listproc.o: listproc.c listproc.h
	gcc -c listproc.c listproc.h
