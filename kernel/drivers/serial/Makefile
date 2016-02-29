include ../../../Makefile.inc
include ../../Makefile_kernel.inc

INCLUDE = -I../../include

all:	serial.a

serial.a:	pl011_uart.o
	$(CROSS)$(AR) crs serial.a pl011_uart.o

pl011_uart.o:	pl011_uart.c
	$(CROSS)$(CC) $(CFLAGS) $(INCLUDE) -o pl011_uart.o -c pl011_uart.c

clean:
	rm -f *~ *.o *.a



