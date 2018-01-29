include ../../../Makefile.inc
include ../../Makefile_kernel.inc

INCLUDE = -I../../include

all:	serial.a

serial.a:	pl011_uart.o mini_uart.o serial.o
	$(CROSS)$(AR) crs serial.a pl011_uart.o mini_uart.o serial.o

pl011_uart.o:	pl011_uart.c
	$(CROSS)$(CC) $(CFLAGS) $(INCLUDE) -o pl011_uart.o -c pl011_uart.c

mini_uart.o:	mini_uart.c
	$(CROSS)$(CC) $(CFLAGS) $(INCLUDE) -o mini_uart.o -c mini_uart.c

serial.o:	serial.c
	$(CROSS)$(CC) $(CFLAGS) $(INCLUDE) -o serial.o -c serial.c

clean:
	rm -f *~ *.o *.a



