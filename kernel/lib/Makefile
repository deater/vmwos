include ../../Makefile.inc
include ../Makefile_kernel.inc

INCLUDE = -I../include

all:	lib-arm1176.a lib-armv7.a

### ARM1176

lib-arm1176.a:	div.arm1176.o memcpy.arm1176.o memory_benchmark.arm1176.o memset.arm1176.o printk.arm1176.o string.arm1176.o
	$(CROSS)$(AR) crs lib-arm1176.a div.arm1176.o memcpy.arm1176.o memory_benchmark.arm1176.o memset.arm1176.o \
		printk.arm1176.o string.arm1176.o

div.arm1176.o:	div.arm1176.c
	$(CROSS)$(CC) $(CFLAGS_ARM1176) $(INCLUDE) -o div.arm1176.o -c div.arm1176.c

memcpy.arm1176.o:	memcpy.c
	$(CROSS)$(CC) $(CFLAGS_ARM1176) $(INCLUDE) -o memcpy.arm1176.o -c memcpy.c

memory_benchmark.arm1176.o:	memory_benchmark.c
	$(CROSS)$(CC) $(CFLAGS_ARM1176) $(INCLUDE) -o memory_benchmark.arm1176.o -c memory_benchmark.c

memset.arm1176.o:	memset.c
	$(CROSS)$(CC) $(CFLAGS_ARM1176) $(INCLUDE) -o memset.arm1176.o -c memset.c

printk.arm1176.o:	printk.c
	$(CROSS)$(CC) $(CFLAGS_ARM1176) $(INCLUDE) -o printk.arm1176.o -c printk.c

string.arm1176.o:	string.c
	$(CROSS)$(CC) $(CFLAGS_ARM1176) $(INCLUDE) -o string.arm1176.o -c string.c

#locks.o:        locks.s
#	$(CROSS)as $(ASFLAGS) -o locks.o locks.s


### ARMV7

lib-armv7.a:	memcpy.armv7.o memory_benchmark.armv7.o memset.armv7.o printk.armv7.o string.armv7.o
	$(CROSS)$(AR) crs lib-armv7.a memcpy.armv7.o memory_benchmark.armv7.o memset.armv7.o \
		printk.armv7.o string.armv7.o

memcpy.armv7.o:	memcpy.c
	$(CROSS)$(CC) $(CFLAGS_ARMV7) $(INCLUDE) -o memcpy.armv7.o -c memcpy.c

memory_benchmark.armv7.o:	memory_benchmark.c
	$(CROSS)$(CC) $(CFLAGS_ARMV7) $(INCLUDE) -o memory_benchmark.armv7.o -c memory_benchmark.c

memset.armv7.o:	memset.c
	$(CROSS)$(CC) $(CFLAGS_ARMV7) $(INCLUDE) -o memset.armv7.o -c memset.c

printk.armv7.o:	printk.c
	$(CROSS)$(CC) $(CFLAGS_ARMV7) $(INCLUDE) -o printk.armv7.o -c printk.c

string.armv7.o:	string.c
	$(CROSS)$(CC) $(CFLAGS_ARMV7) $(INCLUDE) -o string.armv7.o -c string.c

###

clean:
	rm -f *~ *.o *.a

