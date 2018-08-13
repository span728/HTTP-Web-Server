CC = gcc 
CFLAGS = -g -Wall
LDFLAGS = -g

http-server: http-server.o 

http-server.o: http-server.c

.PHONY: clean 
clean: 
	rm -f *.o a.out core http-server

.PHONY: all run free
all: clean http-server 

run: all
	./http-server 31507 ~/html localhost 44507 || ./http-server 31508 ~/html localhost 44507
free: all
	valgrind --leak-check=yes ./http-server 31507 ~/html localhost 44507
