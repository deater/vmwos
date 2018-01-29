include ../../../Makefile.inc
include ../../Makefile_kernel.inc

INCLUDE = -I../../include

all:	bcm2835.a

bcm2835.a:	bcm2835_io.o
	$(CROSS)$(AR) crs bcm2835.a bcm2835_io.o

bcm2835_io.o:	bcm2835_io.c
	$(CROSS)$(CC) $(CFLAGS) $(INCLUDE) -o bcm2835_io.o -c bcm2835_io.c

clean:
	rm -f *~ *.o *.a



