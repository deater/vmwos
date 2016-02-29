include ../../../Makefile.inc
include ../../Makefile_kernel.inc

INCLUDE = -I../../include

all:	thermal.a

thermal.a:	thermal.o
	$(CROSS)$(AR) crs thermal.a thermal.o

thermal.o:	thermal.c
	$(CROSS)$(CC) $(CFLAGS) $(INCLUDE) -o thermal.o -c thermal.c

clean:
	rm -f *~ *.o *.a



