include ../../../Makefile.inc
include ../../Makefile_kernel.inc

INCLUDE = -I../../include

all:	led.a

led.a:	led.o emergency_blink.o
	$(CROSS)$(AR) crs led.a led.o emergency_blink.o

led.o:	led.c
	$(CROSS)$(CC) $(CFLAGS) $(INCLUDE) -o led.o -c led.c

emergency_blink.o:	emergency_blink.c
	$(CROSS)$(CC) $(CFLAGS) $(INCLUDE) -o emergency_blink.o -c emergency_blink.c

clean:
	rm -f *~ *.o *.a



