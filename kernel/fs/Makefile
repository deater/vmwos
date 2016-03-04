include ../../Makefile.inc
include ../Makefile_kernel.inc

INCLUDE = -I../include

all:	fs.a

fs.a:	files.o \
	romfs/romfs.a
	$(CROSS)$(AR) crsT fs.a \
		files.o \
		romfs/romfs.a

files.o:	files.c
	$(CROSS)$(CC) $(CFLAGS) $(INCLUDE) -c files.c

romfs/romfs.a:	force_look
	cd romfs && make

force_look:
	true


clean:
	rm -f *~ *.o *.a
	cd romfs && make clean
