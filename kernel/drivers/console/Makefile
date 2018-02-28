include ../../../Makefile.inc
include ../../Makefile_kernel.inc

INCLUDE = -I../../include

all:	console.arm1176.a console.armv7.a

#### ARM1176

console.arm1176.a:	console_io.arm1176.o
	$(CROSS)$(AR) crus console.arm1176.a console_io.arm1176.o
	$(CROSS)$(RANLIB) console.arm1176.a

console_io.arm1176.o:	console_io.c
	$(CROSS)$(CC) $(CFLAGS_ARM1176) $(INCLUDE) -o console_io.arm1176.o -c console_io.c

#### ARMv7

console.armv7.a:	console_io.armv7.o
	$(CROSS)$(AR) crus console.armv7.a console_io.armv7.o
	$(CROSS)$(RANLIB) console.armv7.a

console_io.armv7.o:	console_io.c
	$(CROSS)$(CC) $(CFLAGS_ARMV7) $(INCLUDE) -o console_io.armv7.o -c console_io.c


####

clean:
	rm -f *~ *.o *.a



