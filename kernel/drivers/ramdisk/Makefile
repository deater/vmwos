include ../../../Makefile.inc
include ../../Makefile_kernel.inc

INCLUDE = -I../../include

all:	ramdisk.arm1176.a ramdisk.armv7.a

### ARM1176

ramdisk.arm1176.a:	ramdisk.arm1176.o
	$(CROSS)$(AR) crus ramdisk.arm1176.a ramdisk.arm1176.o
	$(CROSS)$(RANLIB) ramdisk.arm1176.a

ramdisk.arm1176.o:	ramdisk.c
	$(CROSS)$(CC) $(CFLAGS_ARM1176) $(INCLUDE) -o ramdisk.arm1176.o -c ramdisk.c

### ARMV7

ramdisk.armv7.a:	ramdisk.armv7.o
	$(CROSS)$(AR) crus ramdisk.armv7.a ramdisk.armv7.o
	$(CROSS)$(RANLIB) ramdisk.armv7.a

ramdisk.armv7.o:	ramdisk.c
	$(CROSS)$(CC) $(CFLAGS_ARMV7) $(INCLUDE) -o ramdisk.armv7.o -c ramdisk.c

clean:
	rm -f *~ *.o *.a



