CC = gcc
LFLAGS =
CFLAGS = -O2 -Wall

all:	elf2bflt

elf2bflt:	elf2bflt.o
	$(CC) $(LFLAGS) -o elf2bflt elf2bflt.o

elf2bflt.o:	elf2bflt.c
	$(CC) $(CFLAGS) -c elf2bflt.c

clean:	
	rm -f elf2bflt *~ *.o
