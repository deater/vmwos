include ../../Makefile.inc
include ../Makefile_kernel.inc

all:	fs.a

fs.a:	romfs/romfs.a
	$(CROSS)$(AR) crsT fs.a \
		romfs/romfs.a

romfs/romfs.a:	force_look
	cd romfs && make

force_look:
	true


clean:
	rm -f *~ *.o *.a
	cd romfs && make clean
