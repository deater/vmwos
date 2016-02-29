include ../../../Makefile.inc
include ../../Makefile_kernel.inc

INCLUDE = -I../../include

all:	led.a

led.a:	led.o
	$(CROSS)$(AR) crs led.a led.o

led.o:	led.c
	$(CROSS)$(CC) $(CFLAGS) $(INCLUDE) -o led.o -c led.c

clean:
	rm -f *~ *.o *.a



