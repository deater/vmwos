include ../../../Makefile.inc
include ../../Makefile_kernel.inc

INCLUDE = -I../../include

all:	i2c.armv1176.a i2c.armv7.a

#### ARM1176

i2c.armv1176.a:	bcm2835_i2c.arm1176.o i2c.arm1176.o
	$(CROSS)$(AR) crs i2c.arm1176.a bcm2835_i2c.arm1176.o i2c.arm1176.o

bcm2835_i2c.arm1176.o:	bcm2835_i2c.c
	$(CROSS)$(CC) $(CFLAGS_ARM1176) $(INCLUDE) -o bcm2835_i2c.arm1176.o -c bcm2835_i2c.c

i2c.arm1176.o:	i2c.c
	$(CROSS)$(CC) $(CFLAGS_ARM1176) $(INCLUDE) -o i2c.arm1176.o -c i2c.c

#### ARMV7

i2c.armv7.a:	bcm2835_i2c.armv7.o i2c.armv7.o
	$(CROSS)$(AR) crs i2c.armv7.a bcm2835_i2c.armv7.o i2c.armv7.o

bcm2835_i2c.armv7.o:	bcm2835_i2c.c
	$(CROSS)$(CC) $(CFLAGS_ARMV7) $(INCLUDE) -o bcm2835_i2c.armv7.o -c bcm2835_i2c.c

i2c.armv7.o:	i2c.c
	$(CROSS)$(CC) $(CFLAGS_ARMV7) $(INCLUDE) -o i2c.armv7.o -c i2c.c

clean:
	rm -f *~ *.o *.a



