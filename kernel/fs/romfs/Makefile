include ../../../Makefile.inc
include ../../Makefile_kernel.inc

INCLUDE = -I../../include

all:	romfs-arm1176.a romfs-armv7.a

### ARM1176

romfs-arm1176.a:	romfs.arm1176.o
	$(CROSS)$(AR) crs romfs-arm1176.a romfs.arm1176.o

romfs.arm1176.o:	romfs.c
	$(CROSS)$(CC) $(CFLAGS_ARM1176) $(INCLUDE) -o romfs.arm1176.o -c romfs.c

### ARMV7

romfs-armv7.a:	romfs.armv7.o
	$(CROSS)$(AR) crs romfs-armv7.a romfs.armv7.o

romfs.armv7.o:	romfs.c
	$(CROSS)$(CC) $(CFLAGS_ARMV7) $(INCLUDE) -o romfs.armv7.o -c romfs.c

clean:
	rm -f *~ *.o *.a

