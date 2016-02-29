include ../../Makefile.inc
include ../Makefile_kernel.inc

INCLUDE = -I../include

all:	lib.a

lib.a:	div.o printk.o string.o
	$(CROSS)$(AR) crs lib.a div.o printk.o string.o

div.o:	div.c
	$(CROSS)$(CC) $(CFLAGS) $(INCLUDE) -o div.o -c div.c

printk.o:	printk.c
	$(CROSS)$(CC) $(CFLAGS) $(INCLUDE) -o printk.o -c printk.c

string.o:	string.c
	$(CROSS)$(CC) $(CFLAGS) $(INCLUDE) -o string.o -c string.c

clean:
	rm -f *~ *.o *.a

