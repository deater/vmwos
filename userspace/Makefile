CROSS = arm-none-eabi-
CC = gcc
AS = as
ASFLAGS = -mcpu=arm1176jzf-s
CFLAGS = -O2 -Wall -mcpu=arm1176jzf-s -marm -nostartfiles -ffreestanding -nostdlib
NATIVE_CFLAGS = -O2 -Wall -nostartfiles -ffreestanding -nostdlib -fPIC -g

all:	shell printa

shell:	shell.o vlibc.o syscalls.o
	$(CC) $(NATIVE_CFLAGS) -o shell shell.o vlibc.o syscalls.o

printa:	printa.o vlibc.o syscalls.o
	$(CC) $(NATIVE_CFLAGS) -o printa printa.o vlibc.o syscalls.o

####

printa.o:	printa.c
	$(CC) $(NATIVE_CFLAGS) -c printa.c

shell.o:	shell.c
	$(CC) $(NATIVE_CFLAGS) -c shell.c

vlibc.o:	vlibc.c
	$(CC) $(NATIVE_CFLAGS) -c vlibc.c

syscalls.o:	syscalls.c
	$(CC) $(NATIVE_CFLAGS) -c syscalls.c




clean:
	rm -f *~ *.o shell printa
