include ../../Makefile.inc
include ../Makefile_kernel.inc

INCLUDE = -I../include

all:	debug.a

debug.a:	panic.o early_debug.o emergency_blink.o
	$(CROSS)$(AR) crs debug.a panic.o early_debug.o emergency_blink.o

panic.o:	panic.c
	$(CROSS)$(CC) $(CFLAGS) $(INCLUDE) -o panic.o -c panic.c

early_debug.o:	early_debug.c
	$(CROSS)$(CC) $(CFLAGS) $(INCLUDE) -o early_debug.o -c early_debug.c

emergency_blink.o:	emergency_blink.c
	$(CROSS)$(CC) $(CFLAGS) $(INCLUDE) -o emergency_blink.o -c emergency_blink.c

clean:
	rm -f *~ *.o *.a
