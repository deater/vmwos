include ../Makefile.inc

CC = gcc
AS = as
ASFLAGS = -mcpu=arm1176jzf-s
CFLAGS = -O2 -Wall -DVMWOS=1 -mcpu=arm1176jzf-s -marm -nostartfiles -ffreestanding -nostdlib -fPIC
OBJCOPYFLAGS = -R .note -R .comment -R .note.gnu.build-id
LFLAGS = -T../kernel/kernel.ld

all:	shell cat cat_count hello ll ls printa printb sysinfo uname


image:	shell cat cat_count hello ll ls printa printb sysinfo uname
	mkdir -p image
	mkdir -p image/ansis
	cp shell cat cat_count printa printb sysinfo uname hello ll ls CATME aitasad1.ans utopia3.ans ./image
	cp utopia3.ans image/ansis/other_utopia.ans
	genromfs -f initrd.romfs -d image -V VMWos
	echo "unsigned char initrd_image[]={" > initrd.h
	xxd -i < initrd.romfs >> initrd.h
	echo "};" >> initrd.h

shell:	shell.elf
	$(CROSS)objcopy $(OBJCOPYFLAGS) shell.elf -O binary shell

cat:	cat.elf
	$(CROSS)objcopy $(OBJCOPYFLAGS) cat.elf -O binary cat

cat_count:	cat_count.elf
	$(CROSS)objcopy $(OBJCOPYFLAGS) cat_count.elf -O binary cat_count

hello:	hello.elf
	$(CROSS)objcopy $(OBJCOPYFLAGS) hello.elf -O binary hello

ls:	ls.elf
	$(CROSS)objcopy $(OBJCOPYFLAGS) ls.elf -O binary ls

ll:	ll.elf
	$(CROSS)objcopy $(OBJCOPYFLAGS) ll.elf -O binary ll

printa:	printa.elf
	$(CROSS)objcopy $(OBJCOPYFLAGS) printa.elf -O binary printa

printb:	printb.elf
	$(CROSS)objcopy $(OBJCOPYFLAGS) printb.elf -O binary printb

sysinfo:	sysinfo.elf
	$(CROSS)objcopy $(OBJCOPYFLAGS) sysinfo.elf -O binary sysinfo

uname:	uname.elf
	$(CROSS)objcopy $(OBJCOPYFLAGS) uname.elf -O binary uname



shell.elf:	shell.o start.o vlibc.o syscalls.arm.o vmwos.o
	$(CROSS)$(CC) $(CFLAGS) $(LFLAGS) -o shell.elf start.o shell.o vlibc.o syscalls.arm.o vmwos.o

cat.elf:	cat.o start.o vlibc.o syscalls.arm.o
	$(CROSS)$(CC) $(CFLAGS) $(LFLAGS) -o cat.elf start.o cat.o vlibc.o syscalls.arm.o vmwos.o

cat_count.elf:	cat_count.o start.o vlibc.o syscalls.arm.o
	$(CROSS)$(CC) $(CFLAGS) $(LFLAGS) -o cat_count.elf start.o cat_count.o vlibc.o syscalls.arm.o vmwos.o

hello.elf:	hello.o start.o vlibc.o syscalls.arm.o
	$(CROSS)$(CC) $(CFLAGS) $(LFLAGS) -o hello.elf start.o hello.o vlibc.o syscalls.arm.o vmwos.o

ls.elf:	ls.o start.o vlibc.o syscalls.arm.o
	$(CROSS)$(CC) $(CFLAGS) $(LFLAGS) -o ls.elf start.o ls.o vlibc.o syscalls.arm.o vmwos.o

ll.elf:	ll.o
	$(CROSS)$(LD) -o ll.elf ll.o

printa.elf:	printa.o start.o vlibc.o syscalls.arm.o
	$(CROSS)$(CC) $(CFLAGS) $(LFLAGS) -o printa.elf start.o printa.o vlibc.o syscalls.arm.o vmwos.o

printb.elf:	printb.o start.o vlibc.o syscalls.arm.o
	$(CROSS)$(CC) $(CFLAGS) $(LFLAGS) -o printb.elf start.o printb.o vlibc.o syscalls.arm.o vmwos.o

sysinfo.elf:	sysinfo.o start.o vlibc.o syscalls.arm.o
	$(CROSS)$(CC) $(CFLAGS) $(LFLAGS) -o sysinfo.elf start.o sysinfo.o vlibc.o syscalls.arm.o vmwos.o

uname.elf:	uname.o start.o vlibc.o syscalls.arm.o
	$(CROSS)$(CC) $(CFLAGS) $(LFLAGS) -o uname.elf start.o uname.o vlibc.o syscalls.arm.o vmwos.o

####

cat.o:	cat.c
	$(CROSS)$(CC) $(CFLAGS) -c cat.c

cat_count.o:	cat_count.c
	$(CROSS)$(CC) $(CFLAGS) -c cat_count.c

hello.o:	hello.c
	$(CROSS)$(CC) $(CFLAGS) -c hello.c

printa.o:	printa.c
	$(CROSS)$(CC) $(CFLAGS) -c printa.c

printb.o:	printb.c
	$(CROSS)$(CC) $(CFLAGS) -c printb.c

uname.o:	uname.c
	$(CROSS)$(CC) $(CFLAGS) -c uname.c

shell.o:	shell.c
	$(CROSS)$(CC) $(CFLAGS) -c shell.c

sysinfo.o:	sysinfo.c
	$(CROSS)$(CC) $(CFLAGS) -c sysinfo.c

ls.o:	ls.c
	$(CROSS)$(CC) $(CFLAGS) -c ls.c

ll.o:	ll.arm.eabi.s logo.lzss_new
	$(CROSS)$(AS) -defsym FAKE_PROC=1 -o ll.o ll.arm.eabi.s

start.o:	start.c
	$(CROSS)$(CC) $(CFLAGS) -c start.c

vmwos.o:	vmwos.c
	$(CROSS)$(CC) $(CFLAGS) -c vmwos.c

vlibc.o:	vlibc.c
	$(CROSS)$(CC) $(CFLAGS) -c vlibc.c

syscalls.arm.o:	syscalls.arm.c
	$(CROSS)$(CC) $(CFLAGS) -c syscalls.arm.c

clean:
	rm -f *~ *.o *.elf \
		hello cat cat_count ll ls printa printb shell sysinfo uname \
		initrd.h initrd.romfs
