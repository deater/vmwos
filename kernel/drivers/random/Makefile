include ../../../Makefile.inc
include ../../Makefile_kernel.inc

INCLUDE = -I../../include

all:	random.a

random.a:	bcm2835_rng.o
	$(CROSS)$(AR) crs random.a bcm2835_rng.o

bcm2835_rng.o:	bcm2835_rng.c
	$(CROSS)$(CC) $(CFLAGS) $(INCLUDE) -o bcm2835_rng.o -c bcm2835_rng.c

clean:
	rm -f *~ *.o *.a



