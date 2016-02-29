include ../../Makefile.inc
include ../Makefile_kernel.inc

INCLUDE = -I../include

all:	boot.a

boot.a:	atags.o boot.o
	$(CROSS)$(AR) crs boot.a atags.o boot.o


atags.o:	atags.c
	$(CROSS)$(CC) $(CFLAGS) $(INCLUDE) -o atags.o -c atags.c

boot.o:	boot.s
	$(CROSS)as $(ASFLAGS) $(INCLUDE) -o boot.o boot.s

clean:
	rm -f *~ *.o *.a



