

myshell: myshell.c util execute
	gcc myshell.c util.o execute.o -o myshell.o

execute: execute.c
	gcc -c execute.c

util: util.c
	gcc -c util.c

clear:
	rm *.o

.PHONY:
	clear