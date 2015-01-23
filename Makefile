CROSS = arm-none-eabi-
CC = gcc
AS = as
ASFLAGS = -mcpu=arm1176jzf-s
CFLAGS = -O2 -mcpu=arm1176jzf-s -nostartfiles


LINKER_SCRIPT = kernel.ld

all:	kernel.img rpi_blink.dis

kernel.img:	rpi_blink.elf
	$(CROSS)objcopy rpi_blink.elf -O binary kernel.img

rpi_blink.elf:	rpi_blink.o boot.o
	$(CROSS)ld --no-undefined rpi_blink.o boot.o -Map rpi_blink.map -o rpi_blink.elf -T $(LINKER_SCRIPT)

rpi_blink.o:	rpi_blink.c
	$(CROSS)$(CC) $(CFLAGS) -o rpi_blink.o rpi_blink.c

boot.o:	boot.s
	$(CROSS)as $(ASFLAGS) -o boot.o boot.s

rpi_blink.dis:	rpi_blink.elf
	$(CROSS)objdump --disassemble-all rpi_blink.elf > rpi_blink.dis

clean:
	rm -f *~ *.o *.map *.elf kernel.img *.dis

