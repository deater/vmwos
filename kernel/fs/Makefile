include ../../Makefile.inc
include ../Makefile_kernel.inc

INCLUDE = -I../include

# CFLAGS_ARMV7 += -g

all:	fs-arm1176.a fs-armv7.a

### ARM1176

fs-arm1176.a:	files.arm1176.o inodes.arm1176.o superblock.arm1176.o \
	dos33fs/dos33fs-arm1176.a romfs/romfs-arm1176.a
	$(CROSS)$(AR) crsT fs-arm1176.a \
		files.arm1176.o inodes.arm1176.o superblock.arm1176.o \
		dos33fs/dos33fs-arm1176.a \
		romfs/romfs-arm1176.a

files.arm1176.o:	files.c
	$(CROSS)$(CC) $(CFLAGS_ARM1176) $(INCLUDE) -o files.arm1176.o -c files.c

inodes.arm1176.o:	inodes.c
	$(CROSS)$(CC) $(CFLAGS_ARM1176) $(INCLUDE) -o inodes.arm1176.o -c inodes.c

superblock.arm1176.o:	superblock.c
	$(CROSS)$(CC) $(CFLAGS_ARM1176) $(INCLUDE) -o superblock.arm1176.o -c superblock.c


dos33fs/dos33fs-arm1176.a:	force_look
	cd dos33fs && make

romfs/romfs-arm1176.a:	force_look
	cd romfs && make


### ARMV7

fs-armv7.a:	files.armv7.o inodes.armv7.o superblock.armv7.o \
	dos33fs/dos33fs-armv7.a romfs/romfs-armv7.a
	$(CROSS)$(AR) crsT fs-armv7.a \
		files.armv7.o inodes.armv7.o superblock.armv7.o \
		dos33fs/dos33fs-armv7.a \
		romfs/romfs-armv7.a

files.armv7.o:	files.c
	$(CROSS)$(CC) $(CFLAGS_ARMV7) $(INCLUDE) -o files.armv7.o -c files.c

inodes.armv7.o:	inodes.c
	$(CROSS)$(CC) $(CFLAGS_ARMV7) $(INCLUDE) -o inodes.armv7.o -c inodes.c

superblock.armv7.o:	superblock.c
	$(CROSS)$(CC) $(CFLAGS_ARMV7) $(INCLUDE) -o superblock.armv7.o -c superblock.c


dos33fs/dos33fs-armv7.a:	force_look
	cd dos33fs && make

romfs/romfs-armv7.a:	force_look
	cd romfs && make

####

force_look:
	true


clean:
	rm -f *~ *.o *.a
	cd dos33fs && make clean
	cd romfs && make clean
