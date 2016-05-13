include ../Makefile.inc

CC = gcc
AS = as
ASFLAGS = -mcpu=arm1176jzf-s
CFLAGS = -O2 -Wall -DVMWOS=1 -mcpu=arm1176jzf-s -marm -nostartfiles -ffreestanding -nostdlib -fPIC
OBJCOPYFLAGS = -R .note -R .comment -R .note.gnu.build-id
LFLAGS = -T../kernel/kernel.ld

all:	shell cat hello ls printa printb


image:	shell cat hello ls printa printb
	mkdir -p image
	mkdir -p image/ansis
	cp shell cat printa printb hello ls CATME aitasad1.ans utopia3.ans ./image
	cp utopia3.ans image/ansis/other_utopia.ans
	genromfs -f initrd.romfs -d image -V VMWos
	echo "unsigned char initrd_image[]={" > initrd.h
	xxd -i < initrd.romfs >> initrd.h
	echo "};" >> initrd.h

shell:	shell.elf
	$(CROSS)objcopy $(OBJCOPYFLAGS) shell.elf -O binary shell

cat:	cat.elf
	$(CROSS)objcopy $(OBJCOPYFLAGS) cat.elf -O binary cat

hello:	hello.elf
	$(CROSS)objcopy $(OBJCOPYFLAGS) hello.elf -O binary hello

ls:	ls.elf
	$(CROSS)objcopy $(OBJCOPYFLAGS) ls.elf -O binary ls

printa:	printa.elf
	$(CROSS)objcopy $(OBJCOPYFLAGS) printa.elf -O binary printa

printb:	printb.elf
	$(CROSS)objcopy $(OBJCOPYFLAGS) printb.elf -O binary printb



shell.elf:	shell.o start.o vlibc.o syscalls.arm.o vmwos.o
	$(CROSS)$(CC) $(CFLAGS) $(LFLAGS) -o shell.elf start.o shell.o vlibc.o syscalls.arm.o vmwos.o

cat.elf:	cat.o start.o vlibc.o syscalls.arm.o
	$(CROSS)$(CC) $(CFLAGS) $(LFLAGS) -o cat.elf start.o cat.o vlibc.o syscalls.arm.o vmwos.o

hello.elf:	hello.o start.o vlibc.o syscalls.arm.o
	$(CROSS)$(CC) $(CFLAGS) $(LFLAGS) -o hello.elf start.o hello.o vlibc.o syscalls.arm.o vmwos.o

ls.elf:	ls.o start.o vlibc.o syscalls.arm.o
	$(CROSS)$(CC) $(CFLAGS) $(LFLAGS) -o ls.elf start.o ls.o vlibc.o syscalls.arm.o vmwos.o

printa.elf:	printa.o start.o vlibc.o syscalls.arm.o
	$(CROSS)$(CC) $(CFLAGS) $(LFLAGS) -o printa.elf start.o printa.o vlibc.o syscalls.arm.o vmwos.o

printb.elf:	printb.o start.o vlibc.o syscalls.arm.o
	$(CROSS)$(CC) $(CFLAGS) $(LFLAGS) -o printb.elf start.o printb.o vlibc.o syscalls.arm.o vmwos.o

####

cat.o:	cat.c
	$(CROSS)$(CC) $(CFLAGS) -c cat.c

hello.o:	hello.c
	$(CROSS)$(CC) $(CFLAGS) -c hello.c

printa.o:	printa.c
	$(CROSS)$(CC) $(CFLAGS) -c printa.c

printb.o:	printb.c
	$(CROSS)$(CC) $(CFLAGS) -c printb.c

shell.o:	shell.c
	$(CROSS)$(CC) $(CFLAGS) -c shell.c

ls.o:	ls.c
	$(CROSS)$(CC) $(CFLAGS) -c ls.c

start.o:	start.c
	$(CROSS)$(CC) $(CFLAGS) -c start.c

vmwos.o:	vmwos.c
	$(CROSS)$(CC) $(CFLAGS) -c vmwos.c

vlibc.o:	vlibc.c
	$(CROSS)$(CC) $(CFLAGS) -c vlibc.c

syscalls.arm.o:	syscalls.arm.c
	$(CROSS)$(CC) $(CFLAGS) -c syscalls.arm.c

clean:
	rm -f *~ *.o *.elf hello ls printa printb shell initrd.h initrd.romfs
