include ../../../Makefile.inc
include ../../Makefile_kernel.inc

INCLUDE = -I../../include

all:	spi.armv1176.a spi.armv7.a

#### ARM1176

spi.armv1176.a:	bcm2835_spi.arm1176.o spi.arm1176.o
	$(CROSS)$(AR) crs spi.arm1176.a bcm2835_spi.arm1176.o spi.arm1176.o

bcm2835_spi.arm1176.o:	bcm2835_spi.c
	$(CROSS)$(CC) $(CFLAGS_ARM1176) $(INCLUDE) -o bcm2835_spi.arm1176.o -c bcm2835_spi.c

spi.arm1176.o:	spi.c
	$(CROSS)$(CC) $(CFLAGS_ARM1176) $(INCLUDE) -o spi.arm1176.o -c spi.c

#### ARMV7

spi.armv7.a:	bcm2835_spi.armv7.o spi.armv7.o
	$(CROSS)$(AR) crs spi.armv7.a bcm2835_spi.armv7.o spi.armv7.o

bcm2835_spi.armv7.o:	bcm2835_spi.c
	$(CROSS)$(CC) $(CFLAGS_ARMV7) $(INCLUDE) -o bcm2835_spi.armv7.o -c bcm2835_spi.c

spi.armv7.o:	spi.c
	$(CROSS)$(CC) $(CFLAGS_ARMV7) $(INCLUDE) -o spi.armv7.o -c spi.c

clean:
	rm -f *~ *.o *.a



