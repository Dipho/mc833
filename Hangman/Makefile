CC=gcc
FLAGS = -Wall

all : cliente servidor

cliente : cliente.c basic.h socket_helper.o
	$(CC) $(FLAGS) -o cliente cliente.c socket_helper.o -lncurses

servidor : servidor.c basic.h socket_helper.o
	$(CC) $(FLAGS) -o servidor servidor.c socket_helper.o

clean :
	@rm cliente servidor socket_helper.o
