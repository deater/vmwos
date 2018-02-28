include ../../../Makefile.inc
include ../../Makefile_kernel.inc

INCLUDE = -I../../include

all:	block.arm1176.a block.armv7.a

### ARM1176

block.arm1176.a:	ramdisk.arm1176.o
	$(CROSS)$(AR) crus block.arm1176.a ramdisk.arm1176.o
	$(CROSS)$(RANLIB) block.arm1176.a

ramdisk.arm1176.o:	ramdisk.c
	$(CROSS)$(CC) $(CFLAGS_ARM1176) $(INCLUDE) -o ramdisk.arm1176.o -c ramdisk.c

### ARMV7

block.armv7.a:	ramdisk.armv7.o
	$(CROSS)$(AR) crus block.armv7.a ramdisk.armv7.o
	$(CROSS)$(RANLIB) block.armv7.a

ramdisk.armv7.o:	ramdisk.c
	$(CROSS)$(CC) $(CFLAGS_ARMV7) $(INCLUDE) -o ramdisk.armv7.o -c ramdisk.c

clean:
	rm -f *~ *.o *.a



