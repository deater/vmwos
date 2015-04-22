CROSS = arm-none-eabi-
CC = gcc
AS = as
ASFLAGS = -mcpu=arm1176jzf-s
CFLAGS = -O2 -Wall -mcpu=arm1176jzf-s -marm -nostartfiles -ffreestanding -nostdlib -fPIC

all:	shell printa shell.img shell.h

shell.h:	shell.img
	echo "unsigned char shell_binary[]={" > shell.h
	xxd -i < shell.img >> shell.h
	echo "};" >> shell.h

shell.img:	shell
	$(CROSS)objcopy shell -O binary shell.img


shell:	shell.o vlibc.o syscalls.arm.o vmwos.o
	$(CROSS)$(CC) $(CFLAGS) -o shell shell.o vlibc.o syscalls.arm.o vmwos.o

printa:	printa.o vlibc.o syscalls.arm.o
	$(CROSS)$(CC) $(CFLAGS) -o printa printa.o vlibc.o syscalls.arm.o

####

printa.o:	printa.c
	$(CROSS)$(CC) $(CFLAGS) -c printa.c

shell.o:	shell.c
	$(CROSS)$(CC) $(CFLAGS) -c shell.c

vmwos.o:	vmwos.c
	$(CROSS)$(CC) $(CFLAGS) -c vmwos.c

vlibc.o:	vlibc.c
	$(CROSS)$(CC) $(CFLAGS) -c vlibc.c

syscalls.arm.o:	syscalls.arm.c
	$(CROSS)$(CC) $(CFLAGS) -c syscalls.arm.c




clean:
	rm -f *~ *.o *.img shell printa shell.h
