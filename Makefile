

myshell: myshell.cpp parser execute
	g++ myshell.cpp parser.o execute.o -o myshell -std=c++11 -g

parser: parser.cpp
	g++ -c parser.cpp -std=c++11 -g

execute: execute.cpp
	g++ -c execute.cpp -std=c++11 -g


clear:
	rm *.o myshell

.PHONY:
	clear
