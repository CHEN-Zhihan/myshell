

myshell: myshell.c util
	gcc myshell.c util.o -o myshell.o
	
util: util.c
	gcc -c util.c

clear:
	rm *.o

.PHONY:
	clear