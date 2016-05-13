CC = gcc
CFLAGS = -O2 -Wall
LFLAGS =

all:	ansi_parser

ansi_parser:	ansi_parser.o
	$(CC) $(LFLAGS) -o ansi_parser ansi_parser.o

ansi_parser.o:	ansi_parser.c
	$(CC) $(CFAGS) -c ansi_parser.c

clean:	
	rm -f *~ ansi_parser *.o

