include ../../../Makefile.inc
include ../../Makefile_kernel.inc

INCLUDE = -I../../include

all:	console.a

console.a:	console_io.o
	$(CROSS)$(AR) crus console.a console_io.o
	$(CROSS)$(RANLIB) console.a

console_io.o:	console_io.c
	$(CROSS)$(CC) $(CFLAGS) $(INCLUDE) -o console_io.o -c console_io.c

clean:
	rm -f *~ *.o *.a



