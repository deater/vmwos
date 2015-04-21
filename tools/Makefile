CC = gcc
CFLAGS = -Wall -O2
LFLAGS = 

all:	convert_font

convert_font:	convert_font.o
	$(CC) $(LFLAGS) -o convert_font convert_font.o

convert_font.o:	convert_font.c
	$(CC) $(CFLAGS) -c convert_font.c


clean:
	rm -f *~ *.o convert_font

