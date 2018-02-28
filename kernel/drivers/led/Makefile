include ../../../Makefile.inc
include ../../Makefile_kernel.inc

INCLUDE = -I../../include

all:	led.arm1176.a led.armv7.a

### ARM1176

led.arm1176.a:	led.arm1176.o
	$(CROSS)$(AR) crs led.arm1176.a led.arm1176.o

led.arm1176.o:	led.c
	$(CROSS)$(CC) $(CFLAGS_ARM1176) $(INCLUDE) -o led.arm1176.o -c led.c

### ARMV7

led.armv7.a:	led.armv7.o
	$(CROSS)$(AR) crs led.armv7.a led.armv7.o

led.armv7.o:	led.c
	$(CROSS)$(CC) $(CFLAGS_ARMV7) $(INCLUDE) -o led.armv7.o -c led.c

####

clean:
	rm -f *~ *.o *.a

