include ../../../Makefile.inc
include ../../Makefile_kernel.inc

INCLUDE = -I../../include

all:	romfs.a

romfs.a:	romfs.o
	$(CROSS)$(AR) crs romfs.a romfs.o

romfs.o:	romfs.c
	$(CROSS)$(CC) $(CFLAGS) $(INCLUDE) -o romfs.o -c romfs.c

clean:
	rm -f *~ *.o *.a

