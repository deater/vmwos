include ../../../Makefile.inc
include ../../Makefile_kernel.inc

INCLUDE = -I../../include

all:	gpio.arm1176.a gpio.armv7.a

#### ARM1176

gpio.arm1176.a:	gpio.arm1176.o
	$(CROSS)$(AR) crs gpio.arm1176.a gpio.arm1176.o

gpio.arm1176.o:	gpio.c
	$(CROSS)$(CC) $(CFLAGS_ARM1176) $(INCLUDE) -o gpio.arm1176.o -c gpio.c

#### ARMV7

gpio.armv7.a:	gpio.armv7.o
	$(CROSS)$(AR) crs gpio.armv7.a gpio.armv7.o

gpio.armv7.o:	gpio.c
	$(CROSS)$(CC) $(CFLAGS_ARMV7) $(INCLUDE) -o gpio.armv7.o -c gpio.c

####

clean:
	rm -f *~ *.o *.a



