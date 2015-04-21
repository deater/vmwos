CROSS = arm-none-eabi-
CC = gcc
AS = as
ASFLAGS = -mcpu=arm1176jzf-s
CFLAGS = -O2 -Wall -mcpu=arm1176jzf-s -marm -nostartfiles -ffreestanding -nostdlib


LINKER_SCRIPT = kernel.ld

all:	kernel.img kernel.dis

kernel.img:	kernel.elf
	$(CROSS)objcopy kernel.elf -O binary kernel.img



kernel.elf:	kernel_main.o atags.o serial.o boot.o framebuffer.o \
	framebuffer_console.o \
	interrupts.o io.o led.o mailbox.o printk.o syscalls.o timer.o 
	$(CROSS)ld --no-undefined \
		kernel_main.o atags.o serial.o boot.o framebuffer.o \
		framebuffer_console.o \
		interrupts.o io.o led.o mailbox.o printk.o syscalls.o timer.o \
		-Map kernel.map -o kernel.elf -T $(LINKER_SCRIPT)


kernel_main.o:	kernel_main.c
	$(CROSS)$(CC) $(CFLAGS) -o kernel_main.o -c kernel_main.c


atags.o:	atags.c
	$(CROSS)$(CC) $(CFLAGS) -o atags.o -c atags.c

interrupts.o:	interrupts.c
	$(CROSS)$(CC) $(CFLAGS) -o interrupts.o -c interrupts.c

framebuffer.o:	framebuffer.c
	$(CROSS)$(CC) $(CFLAGS) -o framebuffer.o -c framebuffer.c

framebuffer_console.o:	framebuffer_console.c
	$(CROSS)$(CC) $(CFLAGS) -o framebuffer_console.o -c framebuffer_console.c

io.o:	io.c
	$(CROSS)$(CC) $(CFLAGS) -o io.o -c io.c

led.o:	led.c
	$(CROSS)$(CC) $(CFLAGS) -o led.o -c led.c

mailbox.o:	mailbox.c
	$(CROSS)$(CC) $(CFLAGS) -o mailbox.o -c mailbox.c

printk.o:	printk.c
	$(CROSS)$(CC) $(CFLAGS) -o printk.o -c printk.c

serial.o:	serial.c bcm2835_periph.h
	$(CROSS)$(CC) $(CFLAGS) -o serial.o -c serial.c

shell.o:	shell.c
	$(CROSS)$(CC) $(CFLAGS) -o shell.o -c shell.c

syscalls.o:	syscalls.c
	$(CROSS)$(CC) $(CFLAGS) -o syscalls.o -c syscalls.c

timer.o:	timer.c
	$(CROSS)$(CC) $(CFLAGS) -o timer.o -c timer.c

boot.o:	boot.s
	$(CROSS)as $(ASFLAGS) -o boot.o boot.s




kernel.dis:	kernel.elf
	$(CROSS)objdump --disassemble-all kernel.elf > kernel.dis

clean:
	rm -f *~ *.o *.map *.elf kernel.img *.dis


submit:
	tar -czvf hw3_submit.tar.gz *.c *.h *.s *.ld Makefile

