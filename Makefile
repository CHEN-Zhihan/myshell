

myshell: myshell.c util execute parser sig
	gcc myshell.c util.o execute.o parser.o sig.o -o myshell.o

execute: execute.c
	gcc -c execute.c

parser: parser.c
	gcc -c parser.c

util: util.c
	gcc -c util.c

sig: sig.c
	gcc -c sig.c

clear:
	rm *.o

.PHONY:
	clear