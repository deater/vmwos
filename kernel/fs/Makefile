include ../../Makefile.inc
include ../Makefile_kernel.inc

INCLUDE = -I../include

all:	fs-arm1176.a fs-armv7.a

### ARM1176

fs-arm1176.a:	files.arm1176.o \
	romfs/romfs-arm1176.a
	$(CROSS)$(AR) crsT fs-arm1176.a \
		files.arm1176.o \
		romfs/romfs-arm1176.a

files.arm1176.o:	files.c
	$(CROSS)$(CC) $(CFLAGS_ARM1176) $(INCLUDE) -o files.arm1176.o -c files.c

romfs/romfs-arm1176.a:	force_look
	cd romfs && make


### ARMV7

fs-armv7.a:	files.armv7.o \
	romfs/romfs-armv7.a
	$(CROSS)$(AR) crsT fs-armv7.a \
		files.armv7.o \
		romfs/romfs-armv7.a

files.armv7.o:	files.c
	$(CROSS)$(CC) $(CFLAGS_ARMV7) $(INCLUDE) -o files.armv7.o -c files.c

romfs/romfs-armv7.a:	force_look
	cd romfs && make

####

force_look:
	true


clean:
	rm -f *~ *.o *.a
	cd romfs && make clean
