CC = gcc
CFLAGS = -Wall -O2

all:	linux_memset

linux_memset:	linux_memset.o
	$(CC) $(LFLAGS) -o linux_memset linux_memset.o

linux_memset.o:	linux_memset.c
	$(CC) $(CFLAGS) -c linux_memset.c

clean:	
	rm -f *~ *.o linux_memset
