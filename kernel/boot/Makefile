include ../../Makefile.inc
include ../Makefile_kernel.inc

INCLUDE = -I../include

all:	atags.o boot-arm1176.o boot-arm7.o

#boot.a:	atags.o boot.o
#	$(CROSS)$(AR) crs boot.a atags.o boot.o


atags.o:	atags.c
	$(CROSS)$(CC) $(CFLAGS) $(INCLUDE) -o atags.o -c atags.c

boot-arm7.o:	boot-arm7.s
	$(CROSS)as $(ASFLAGS) $(INCLUDE) -o boot-arm7.o boot-arm7.s

boot-arm1176.o:	boot-arm1176.s
	$(CROSS)as $(ASFLAGS) $(INCLUDE) -o boot-arm1176.o boot-arm1176.s

clean:
	rm -f *~ *.o *.a



