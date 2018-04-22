include ../Makefile.inc
include Makefile_kernel.inc

LINKER_SCRIPT = kernel.ld

INCLUDE = -I./include

all:	kernel7.img kernel.img kernel7.dis kernel.dis

kernel.img:	kernel.elf
	$(CROSS)objcopy kernel.elf -O binary kernel.img

kernel7.img:	kernel7.elf
	$(CROSS)objcopy kernel7.elf -O binary kernel7.img

../userspace/initrd.h:
	cd ../userspace && make image

### ARM1176

kernel.elf:	kernel_main.arm1176.o \
	boot/boot-arm1176.a \
	debug/debug-arm1176.a \
	drivers/drivers-arm1176.a \
	fs/fs-arm1176.a \
	interrupts/interrupts-arm1176.a \
	memory/memory-arm1176.a \
	processes/processes-arm1176.a \
	syscalls/syscalls-arm1176.a \
	time/time-arm1176.a \
	lib/lib-arm1176.a
	$(CROSS)ld $(LFLAGS) \
		kernel_main.arm1176.o \
		boot/boot-arm1176.a \
		syscalls/syscalls-arm1176.a \
		interrupts/interrupts-arm1176.a \
		fs/fs-arm1176.a \
		drivers/drivers-arm1176.a \
		debug/debug-arm1176.a \
		memory/memory-arm1176.a \
		processes/processes-arm1176.a \
		time/time-arm1176.a \
		lib/lib-arm1176.a \
		-Map kernel.map -o kernel.elf

kernel_main.arm1176.o:	kernel_main.c version.h ../userspace/initrd.h
	$(CROSS)$(CC) $(CFLAGS_ARM1176) $(INCLUDE) \
		-o kernel_main.arm1176.o -c kernel_main.c

boot/boot-arm1176.a:	force_look
	cd boot && make

debug/debug-arm1176.a:	force_look
	cd debug && make

drivers/drivers-arm1176.a:	force_look
	cd drivers && make

fs/fs-arm1176.a:	force_look
	cd fs && make

interrupts/interrupts-arm1176.a:	force_look
	cd interrupts && make

lib/lib-arm1176.a:	force_look
	cd lib && make

memory/memory-arm1176.a:	force_look
	cd memory && make

processes/processes-arm1176.a:	force_look
	cd processes && make

syscalls/syscalls-arm1176.a:	force_look
	cd syscalls && make

time/time-arm1176.a:	force_look
	cd time && make

### ARMV7

kernel7.elf:	kernel_main.armv7.o \
	boot/boot-armv7.a \
	debug/debug-armv7.a \
	drivers/drivers-armv7.a \
	fs/fs-armv7.a \
	interrupts/interrupts-armv7.a \
	memory/memory-armv7.a \
	processes/processes-armv7.a \
	syscalls/syscalls-armv7.a \
	time/time-armv7.a \
	lib/lib-armv7.a
	$(CROSS)ld $(LFLAGS) \
		kernel_main.armv7.o \
		boot/boot-armv7.a \
		syscalls/syscalls-armv7.a \
		interrupts/interrupts-armv7.a \
		fs/fs-armv7.a \
		drivers/drivers-armv7.a \
		memory/memory-armv7.a \
		processes/processes-armv7.a \
		debug/debug-armv7.a \
		time/time-armv7.a \
		lib/lib-armv7.a \
		-Map kernel7.map -o kernel7.elf

kernel_main.armv7.o:	kernel_main.c version.h ../userspace/initrd.h
	$(CROSS)$(CC) $(CFLAGS_ARMV7) $(INCLUDE) \
		-o kernel_main.armv7.o -c kernel_main.c

boot/boot-armv7.a:	force_look
	cd boot && make

debug/debug-armv7.a:	force_look
	cd debug && make

drivers/drivers-armv7.a:	force_look
	cd drivers && make

fs/fs-armv7.a:	force_look
	cd fs && make

interrupts/interrupts-armv7.a:	force_look
	cd interrupts && make

lib/lib-armv7.a:	force_look
	cd lib && make

memory/memory-armv7.a:	force_look
	cd memory && make

processes/processes-armv7.a:	force_look
	cd processes && make

syscalls/syscalls-armv7.a:	force_look
	cd syscalls && make

time/time-armv7.a:	force_look
	cd time && make

###

force_look:
		true

kernel7.dis:	kernel7.elf
	$(CROSS)objdump --disassemble-all kernel7.elf > kernel7.dis

kernel.dis:	kernel.elf
	$(CROSS)objdump --disassemble-all kernel.elf > kernel.dis

clean:
	rm -f *~ *.o *.map *.elf kernel.img kernel7.img *.dis date.h
	cd boot && make clean
	cd debug && make clean
	cd drivers && make clean
	cd fs && make clean
	cd interrupts && make clean
	cd memory && make clean
	cd processes && make clean
	cd syscalls && make clean
	cd time && make clean
	cd lib && make clean
