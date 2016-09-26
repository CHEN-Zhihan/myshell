CC = gcc

all: loopf forever inproc outproc nested

loopf: loopever.c
	$(CC) $< -D FOREVER=0 -o $@
    
forever: loopever.c
	$(CC) $< -D FOREVER=1 -o $@
    
inproc: inProc.c
	$(CC) $< -o $@

outproc: outProc.c
	$(CC) $< -o $@

nested: nested.c
	$(CC) $< -o $@

exe32: myshell32
	chmod +x myshell32

exe64: myshell64
	chmod +x myshell64

clean:
	rm -f loopf forever inproc outproc nested
