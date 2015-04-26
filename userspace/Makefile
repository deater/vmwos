CROSS = 
#arm-none-eabi-
CC = gcc
AS = as
ASFLAGS = -mcpu=arm1176jzf-s
CFLAGS = -O2 -Wall -mcpu=arm1176jzf-s -marm -nostartfiles -ffreestanding -nostdlib -fPIC
OBJCOPYFLAGS = -R .note -R .comment -R .note.gnu.build-id

all:	shell.h printa.h printb.h

shell.h:	shell.img
	echo "unsigned char shell_binary[]={" > shell.h
	xxd -i < shell.img >> shell.h
	echo "};" >> shell.h

printa.h:	printa.img
	echo "unsigned char printa_binary[]={" > printa.h
	xxd -i < printa.img >> printa.h
	echo "};" >> printa.h

printb.h:	printb.img
	echo "unsigned char printb_binary[]={" > printb.h
	xxd -i < printb.img >> printb.h
	echo "};" >> printb.h

shell.img:	shell
	$(CROSS)objcopy $(OBJCOPYFLAGS) shell -O binary shell.img

printa.img:	printa
	$(CROSS)objcopy $(OBJCOPYFLAGS) printa -O binary printa.img

printb.img:	printb
	$(CROSS)objcopy $(OBJCOPYFLAGS) printb -O binary printb.img


shell:	shell.o vlibc.o syscalls.arm.o vmwos.o
	$(CROSS)$(CC) $(CFLAGS) -o shell shell.o vlibc.o syscalls.arm.o vmwos.o

printa:	printa.o vlibc.o syscalls.arm.o
	$(CROSS)$(CC) $(CFLAGS) -o printa printa.o vlibc.o syscalls.arm.o

printb:	printb.o vlibc.o syscalls.arm.o
	$(CROSS)$(CC) $(CFLAGS) -o printb printb.o vlibc.o syscalls.arm.o

####

printa.o:	printa.c
	$(CROSS)$(CC) $(CFLAGS) -c printa.c

printb.o:	printb.c
	$(CROSS)$(CC) $(CFLAGS) -c printb.c

shell.o:	shell.c
	$(CROSS)$(CC) $(CFLAGS) -c shell.c

vmwos.o:	vmwos.c
	$(CROSS)$(CC) $(CFLAGS) -c vmwos.c

vlibc.o:	vlibc.c
	$(CROSS)$(CC) $(CFLAGS) -c vlibc.c

syscalls.arm.o:	syscalls.arm.c
	$(CROSS)$(CC) $(CFLAGS) -c syscalls.arm.c

clean:
	rm -f *~ *.o *.img shell printa printb shell.h printa.h printb.h
