include ../../Makefile.inc
include ../Makefile_kernel.inc

INCLUDE = -I../include

all:	boot-arm7.a boot-arm1176.a

boot-arm7.a:	atags.o boot-arm7.o device_tree.o
	$(CROSS)$(AR) crs boot-arm7.a atags.o boot-arm7.o device_tree.o

boot-arm1176.a:	atags.o boot-arm1176.o device_tree.o
	$(CROSS)$(AR) crs boot-arm1176.a atags.o boot-arm1176.o device_tree.o

atags.o:	atags.c
	$(CROSS)$(CC) $(CFLAGS) $(INCLUDE) -o atags.o -c atags.c

boot-arm7.o:	boot-arm7.s
	$(CROSS)as $(ASFLAGS) $(INCLUDE) -o boot-arm7.o boot-arm7.s

boot-arm1176.o:	boot-arm1176.s
	$(CROSS)as $(ASFLAGS) $(INCLUDE) -o boot-arm1176.o boot-arm1176.s

device_tree.o:	device_tree.c
	$(CROSS)$(CC) $(CFLAGS) $(INCLUDE) -o device_tree.o -c device_tree.c

clean:
	rm -f *~ *.o *.a



