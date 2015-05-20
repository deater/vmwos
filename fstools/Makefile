CC = gcc
CFLAGS = -Wall -g
LFLAGS =


all:	mkromfs

mkromfs:	mkromfs.o
	$(CC) $(LFLAGS) -o mkromfs mkromfs.o

mkromfs.o:	mkromfs.c
	$(CC) $(CFLAGS) -c mkromfs.c

clean:	
	rm -f *~ *.o mkromfs

