CC = gcc
LFLAGS =
CFLAGS = -O2 -Wall

all:	elf2bflt elf2bflt.old bflt_dump

###

elf2bflt:	elf2bflt.o
	$(CC) $(LFLAGS) -o elf2bflt elf2bflt.o

elf2bflt.o:	elf2bflt.c
	$(CC) $(CFLAGS) -c elf2bflt.c


###

elf2flt.old:	elf2flt.old.o
	$(CC) $(LFLAGS) -o elf2flt.old elf2flt.old.o

elf2flt.old.o:	elf2flt.old.c
	$(CC) $(CFLAGS) -c elf2flt.old.c


###


bflt_dump:	bflt_dump.o
	$(CC) $(LFLAGS) -o bflt_dump bflt_dump.o

bflt_dump.o:	bflt_dump.c
	$(CC) $(CFLAGS) -c bflt_dump.c

###

clean:	
	rm -f elf2bflt elf2bflt.old bflt_dump *~ *.o
