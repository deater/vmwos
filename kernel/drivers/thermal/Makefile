include ../../../Makefile.inc
include ../../Makefile_kernel.inc

INCLUDE = -I../../include

all:	thermal.arm1176.a thermal.armv7.a

#### ARM1176

thermal.arm1176.a:	thermal.arm1176.o
	$(CROSS)$(AR) crs thermal.arm1176.a thermal.arm1176.o

thermal.arm1176.o:	thermal.c
	$(CROSS)$(CC) $(CFLAGS_ARM1176) $(INCLUDE) -o thermal.arm1176.o -c thermal.c

#### ARMV7

thermal.armv7.a:	thermal.armv7.o
	$(CROSS)$(AR) crs thermal.armv7.a thermal.armv7.o

thermal.armv7.o:	thermal.c
	$(CROSS)$(CC) $(CFLAGS_ARMV7) $(INCLUDE) -o thermal.armv7.o -c thermal.c

####

clean:
	rm -f *~ *.o *.a



