

myshell: myshell.c util execute parser
	gcc myshell.c util.o execute.o parser.o -o myshell.o

execute: execute.c
	gcc -c execute.c

parser: parser.c
	gcc -c parser.c

util: util.c
	gcc -c util.c

clear:
	rm *.o

.PHONY:
	clear