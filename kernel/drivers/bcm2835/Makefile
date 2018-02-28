include ../../../Makefile.inc
include ../../Makefile_kernel.inc

INCLUDE = -I../../include

all:	bcm2835.arm1176.a bcm2835.armv7.a

bcm2835.arm1176.a:	bcm2835_io.arm1176.o
	$(CROSS)$(AR) crs bcm2835.arm1176.a bcm2835_io.arm1176.o

bcm2835.armv7.a:	bcm2835_io.armv7.o
	$(CROSS)$(AR) crs bcm2835.armv7.a bcm2835_io.armv7.o

###

bcm2835_io.arm1176.o:	bcm2835_io.c
	$(CROSS)$(CC) $(CFLAGS_ARM1176) $(INCLUDE) -o bcm2835_io.arm1176.o -c bcm2835_io.c

bcm2835_io.armv7.o:	bcm2835_io.c
	$(CROSS)$(CC) $(CFLAGS_ARMV7) $(INCLUDE) -o bcm2835_io.armv7.o -c bcm2835_io.c

clean:
	rm -f *~ *.o *.a



