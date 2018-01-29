include ../../Makefile.inc
include ../Makefile_kernel.inc

INCLUDE = -I../include

all:	boot-arm7.a boot-arm1176.a

boot-arm7.a:	atags.o boot-arm7.o
	$(CROSS)$(AR) crs boot-arm7.a atags.o boot-arm7.o

boot-arm1176.a:	atags.o boot-arm1176.o
	$(CROSS)$(AR) crs boot-arm1176.a atags.o boot-arm1176.o

atags.o:	atags.c
	$(CROSS)$(CC) $(CFLAGS) $(INCLUDE) -o atags.o -c atags.c

boot-arm7.o:	boot-arm7.s
	$(CROSS)as $(ASFLAGS) $(INCLUDE) -o boot-arm7.o boot-arm7.s

boot-arm1176.o:	boot-arm1176.s
	$(CROSS)as $(ASFLAGS) $(INCLUDE) -o boot-arm1176.o boot-arm1176.s

clean:
	rm -f *~ *.o *.a



