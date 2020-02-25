include ../../Makefile.inc
include ../Makefile_kernel.inc

INCLUDE = -I../include

all:	lib-arm1176.a lib-armv7.a

### ARM1176

lib-arm1176.a:	div.arm1176.o locks.arm1176.o \
		memcpy.arm1176.o memset.arm1176.o printk.arm1176.o \
		dmesg.arm1176.o \
		smp.arm1176.o string.arm1176.o
	$(CROSS)$(AR) crs lib-arm1176.a div.arm1176.o locks.arm1176.o \
		memcpy.arm1176.o memset.arm1176.o printk.arm1176.o \
		dmesg.arm1176.o \
		smp.arm1176.o string.arm1176.o

div.arm1176.o:	div.arm1176.c
	$(CROSS)$(CC) $(CFLAGS_ARM1176) $(INCLUDE) -o div.arm1176.o -c div.arm1176.c

#memcpy.arm1176.o:	memcpy.c
#	$(CROSS)$(CC) $(CFLAGS_ARM1176) $(INCLUDE) -o memcpy.arm1176.o -c memcpy.c

memcpy.arm1176.o:	memcpy.s
	$(CROSS)$(AS) $(ASFLAGS) -o memcpy.arm1176.o -c memcpy.s


memset.arm1176.o:	memset.c
	$(CROSS)$(CC) $(CFLAGS_ARM1176) $(INCLUDE) -o memset.arm1176.o -c memset.c

dmesg.arm1176.o:	dmesg.c
	$(CROSS)$(CC) $(CFLAGS_ARM1176) $(INCLUDE) -o dmesg.arm1176.o -c dmesg.c

printk.arm1176.o:	printk.c
	$(CROSS)$(CC) $(CFLAGS_ARM1176) $(INCLUDE) -o printk.arm1176.o -c printk.c

smp.arm1176.o:	smp.c
	$(CROSS)$(CC) $(CFLAGS_ARM1176) $(INCLUDE) -o smp.arm1176.o -c smp.c

string.arm1176.o:	string.c
	$(CROSS)$(CC) $(CFLAGS_ARM1176) $(INCLUDE) -o string.arm1176.o -c string.c

locks.arm1176.o:        locks.arm1176.s
	$(CROSS)$(AS) $(ASFLAGS) -o locks.arm1176.o locks.arm1176.s


### ARMV7

lib-armv7.a:	locks.armv7.o memcpy.armv7.o memset.armv7.o \
		dmesg.armv7.o printk.armv7.o smp.armv7.o string.armv7.o
	$(CROSS)$(AR) crs lib-armv7.a \
		locks.armv7.o memcpy.armv7.o memset.armv7.o \
		dmesg.armv7.o printk.armv7.o smp.armv7.o string.armv7.o

locks.armv7.o:	locks.armv7.s
	$(CROSS)$(AS) $(ASFLAGS_ARMV7) $(INCLUDE) -o locks.armv7.o locks.armv7.s

#memcpy.armv7.o:	memcpy.c
#	$(CROSS)$(CC) $(CFLAGS_ARMV7) $(INCLUDE) -o memcpy.armv7.o -c memcpy.c

memcpy.armv7.o:	memcpy.s
	$(CROSS)$(AS) $(ASFLAGS_ARMV7) $(INCLUDE) -o memcpy.armv7.o -c memcpy.s

memset.armv7.o:	memset.c
	$(CROSS)$(CC) $(CFLAGS_ARMV7) $(INCLUDE) -o memset.armv7.o -c memset.c

dmesg.armv7.o:	dmesg.c
	$(CROSS)$(CC) $(CFLAGS_ARMV7) $(INCLUDE) -o dmesg.armv7.o -c dmesg.c

printk.armv7.o:	printk.c
	$(CROSS)$(CC) $(CFLAGS_ARMV7) $(INCLUDE) -o printk.armv7.o -c printk.c

smp.armv7.o:	smp.c
	$(CROSS)$(CC) $(CFLAGS_ARMV7) $(INCLUDE) -o smp.armv7.o -c smp.c

string.armv7.o:	string.c
	$(CROSS)$(CC) $(CFLAGS_ARMV7) $(INCLUDE) -o string.armv7.o -c string.c

###

clean:
	rm -f *~ *.o *.a

