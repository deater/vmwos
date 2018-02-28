include ../../../Makefile.inc
include ../../Makefile_kernel.inc

INCLUDE = -I../../include

all:	serial.armv1176.a serial.armv7.a

#### ARM1176

serial.armv1176.a:	pl011_uart.arm1176.o mini_uart.arm1176.o serial.arm1176.o
	$(CROSS)$(AR) crs serial.arm1176.a pl011_uart.arm1176.o mini_uart.arm1176.o serial.arm1176.o

pl011_uart.arm1176.o:	pl011_uart.c
	$(CROSS)$(CC) $(CFLAGS_ARM1176) $(INCLUDE) -o pl011_uart.arm1176.o -c pl011_uart.c

mini_uart.arm1176.o:	mini_uart.c
	$(CROSS)$(CC) $(CFLAGS_ARM1176) $(INCLUDE) -o mini_uart.arm1176.o -c mini_uart.c

serial.arm1176.o:	serial.c
	$(CROSS)$(CC) $(CFLAGS_ARM1176) $(INCLUDE) -o serial.arm1176.o -c serial.c

#### ARMV7

serial.armv7.a:	pl011_uart.armv7.o mini_uart.armv7.o serial.armv7.o
	$(CROSS)$(AR) crs serial.armv7.a pl011_uart.armv7.o mini_uart.armv7.o serial.armv7.o

pl011_uart.armv7.o:	pl011_uart.c
	$(CROSS)$(CC) $(CFLAGS_ARMV7) $(INCLUDE) -o pl011_uart.armv7.o -c pl011_uart.c

mini_uart.armv7.o:	mini_uart.c
	$(CROSS)$(CC) $(CFLAGS_ARMV7) $(INCLUDE) -o mini_uart.armv7.o -c mini_uart.c

serial.armv7.o:	serial.c
	$(CROSS)$(CC) $(CFLAGS_ARMV7) $(INCLUDE) -o serial.armv7.o -c serial.c

clean:
	rm -f *~ *.o *.a



