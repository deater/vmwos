include ../../../Makefile.inc
include ../../Makefile_kernel.inc

INCLUDE = -I../../include

all:	block.a

block.a:	ramdisk.o
	$(CROSS)$(AR) crus block.a ramdisk.o
	$(CROSS)$(RANLIB) block.a

ramdisk.o:	ramdisk.c
	$(CROSS)$(CC) $(CFLAGS) $(INCLUDE) -o ramdisk.o -c ramdisk.c

clean:
	rm -f *~ *.o *.a



