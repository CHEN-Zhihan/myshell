

myshell: myshell.c util execute parser sig viewtree
	gcc myshell.c util.o execute.o parser.o sig.o viewtree -o myshell.o

execute: execute.c
	gcc -c execute.c

parser: parser.c
	gcc -c parser.c

util: util.c
	gcc -c util.c

sig: sig.c
	gcc -c sig.c

viewtree: viewtree.c
	gcc -c viewtree.c

clear:
	rm *.o

.PHONY:
	clear