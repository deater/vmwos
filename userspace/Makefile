include ../Makefile.inc

CC = gcc
AS = as
ASFLAGS = -mcpu=arm1176jzf-s
CFLAGS = -O2 -Wall -mcpu=arm1176jzf-s -marm -nostartfiles -ffreestanding -nostdlib -fPIC
OBJCOPYFLAGS = -R .note -R .comment -R .note.gnu.build-id

all:	shell hello ls printa printb


image:	shell hello ls printa printb
	mkdir -p image
	cp shell printa printb hello ls CATME aitasad1.ans utopia3.ans ./image
	genromfs -f initrd.romfs -d image -V VMWos
	echo "unsigned char initrd_image[]={" > initrd.h
	xxd -i < initrd.romfs >> initrd.h
	echo "};" >> initrd.h

shell:	shell.elf
	$(CROSS)objcopy $(OBJCOPYFLAGS) shell.elf -O binary shell

hello:	hello.elf
	$(CROSS)objcopy $(OBJCOPYFLAGS) hello.elf -O binary hello

ls:	ls.elf
	$(CROSS)objcopy $(OBJCOPYFLAGS) ls.elf -O binary ls

printa:	printa.elf
	$(CROSS)objcopy $(OBJCOPYFLAGS) printa.elf -O binary printa

printb:	printb.elf
	$(CROSS)objcopy $(OBJCOPYFLAGS) printb.elf -O binary printb



shell.elf:	shell.o vlibc.o syscalls.arm.o vmwos.o
	$(CROSS)$(CC) $(CFLAGS) -o shell.elf shell.o vlibc.o syscalls.arm.o vmwos.o

hello.elf:	hello.o vlibc.o syscalls.arm.o
	$(CROSS)$(CC) $(CFLAGS) -o hello.elf hello.o vlibc.o syscalls.arm.o vmwos.o

ls.elf:	ls.o vlibc.o syscalls.arm.o
	$(CROSS)$(CC) $(CFLAGS) -o ls.elf ls.o vlibc.o syscalls.arm.o vmwos.o

printa.elf:	printa.o vlibc.o syscalls.arm.o
	$(CROSS)$(CC) $(CFLAGS) -o printa.elf printa.o vlibc.o syscalls.arm.o vmwos.o

printb.elf:	printb.o vlibc.o syscalls.arm.o
	$(CROSS)$(CC) $(CFLAGS) -o printb.elf printb.o vlibc.o syscalls.arm.o vmwos.o

####

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

vmwos.o:	vmwos.c
	$(CROSS)$(CC) $(CFLAGS) -c vmwos.c

vlibc.o:	vlibc.c
	$(CROSS)$(CC) $(CFLAGS) -c vlibc.c

syscalls.arm.o:	syscalls.arm.c
	$(CROSS)$(CC) $(CFLAGS) -c syscalls.arm.c

clean:
	rm -f *~ *.o *.elf hello ls printa printb shell initrd.h initrd.romfs
