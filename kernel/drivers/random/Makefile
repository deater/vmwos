include ../../../Makefile.inc
include ../../Makefile_kernel.inc

INCLUDE = -I../../include

all:	random.arm1176.a random.armv7.a

### ARM1176

random.arm1176.a:	bcm2835_rng.arm1176.o
	$(CROSS)$(AR) crs random.arm1176.a bcm2835_rng.arm1176.o

bcm2835_rng.arm1176.o:	bcm2835_rng.c
	$(CROSS)$(CC) $(CFLAGS_ARM1176) $(INCLUDE) -o bcm2835_rng.arm1176.o -c bcm2835_rng.c

### ARMV7

random.armv7.a:	bcm2835_rng.armv7.o
	$(CROSS)$(AR) crs random.armv7.a bcm2835_rng.armv7.o

bcm2835_rng.armv7.o:	bcm2835_rng.c
	$(CROSS)$(CC) $(CFLAGS_ARMV7) $(INCLUDE) -o bcm2835_rng.armv7.o -c bcm2835_rng.c

####

clean:
	rm -f *~ *.o *.a



