CC = gcc
CFLAGS = -Wall -g
LFLAGS =

all:	vmwasm

vmwasm:	vmwasm.o
	$(CC) $(LFLAGS) -o vmwasm vmwasm.o

vmwasm.o:	vmwasm.c
	$(CC) $(CFLAGS) -c vmwasm.c

clean:
	rm -f *.o *~ vmwasm

