CC = gcc
CFLAGS = -Wall -O2 -g
LFLAGS =


all:	mkromfs dumpromfs

mkromfs:	mkromfs.o
	$(CC) $(LFLAGS) -o mkromfs mkromfs.o

mkromfs.o:	mkromfs.c
	$(CC) $(CFLAGS) -c mkromfs.c

dumpromfs:	dumpromfs.o
	$(CC) $(LFLAGS) -o dumpromfs dumpromfs.o

dumpromfs.o:	dumpromfs.c
	$(CC) $(CFLAGS) -c dumpromfs.c

clean:	
	rm -f *~ *.o mkromfs dumpromfs

