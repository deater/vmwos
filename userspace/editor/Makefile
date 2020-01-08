CC = gcc
CFLAGS = -Wall -O2 -g

all:	nano

nano:	nano.o
	$(CC) -o nano nano.o $(LFLAGS)

nano.o:	nano.c
	$(CC) $(CFLAGS) -c nano.c

clean:	
	rm -f *.o *~ nano

