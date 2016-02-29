include ../../../Makefile.inc
include ../../Makefile_kernel.inc

INCLUDE = -I../../include

all:	gpio.a

gpio.a:	gpio.o
	$(CROSS)$(AR) crs gpio.a gpio.o

gpio.o:	gpio.c
	$(CROSS)$(CC) $(CFLAGS) $(INCLUDE) -o gpio.o -c gpio.c

clean:
	rm -f *~ *.o *.a



