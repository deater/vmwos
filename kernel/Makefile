include ../Makefile.inc
include Makefile_kernel.inc

LINKER_SCRIPT = kernel.ld

INCLUDE = -I./include

all:	kernel7.img kernel.img kernel7.dis kernel.dis

kernel.img:	kernel.elf
	$(CROSS)objcopy kernel.elf -O binary kernel.img

kernel7.img:	kernel7.elf
	$(CROSS)objcopy kernel7.elf -O binary kernel7.img

kernel.elf:	kernel_main.o \
	idle_task.o syscalls.o process.o scheduler.o time.o wait.o \
	swi_handler.o \
	boot/boot-arm1176.a \
	debug/debug-arm1176.a \
	drivers/drivers.arm1176.a \
	fs/fs.a \
	interrupts/interrupts-arm1176.a \
	memory/memory-arm1176.a \
	syscalls/syscalls.a \
	lib/lib-arm1176.a
	$(CROSS)ld $(LFLAGS) \
		kernel_main.o \
		idle_task.o \
		syscalls.o process.o scheduler.o time.o wait.o \
		swi_handler.o \
		boot/boot-arm1176.a \
		fs/fs.a \
		interrupts/interrupts-arm1176.a \
		drivers/drivers.arm1176.a \
		debug/debug-arm1176.a \
		memory/memory-arm1176.a \
		syscalls/syscalls.a \
		lib/lib-arm1176.a \
		-Map kernel.map -o kernel.elf

kernel7.elf:	kernel_main.o \
	idle_task.o syscalls.o process.o scheduler.o time.o wait.o \
	swi_handler.o \
	boot/boot-armv7.a \
	debug/debug-armv7.a \
	drivers/drivers.armv7.a \
	fs/fs.a \
	interrupts/interrupts-armv7.a \
	memory/memory-armv7.a \
	syscalls/syscalls.a \
	lib/lib-armv7.a
	$(CROSS)ld $(LFLAGS) \
		kernel_main.o idle_task.o syscalls.o \
		process.o scheduler.o time.o wait.o swi_handler.o \
		boot/boot-armv7.a \
		fs/fs.a \
		interrupts/interrupts-armv7.a \
		drivers/drivers.armv7.a \
		debug/debug-armv7.a \
		memory/memory-armv7.a \
		syscalls/syscalls.a \
		lib/lib-armv7.a \
		-Map kernel7.map -o kernel7.elf

kernel_main.o:	kernel_main.c version.h
	$(CROSS)$(CC) $(CFLAGS) $(INCLUDE) -o kernel_main.o -c kernel_main.c

idle_task.o:	idle_task.c
	$(CROSS)$(CC) $(CFLAGS) -o idle_task.o -c idle_task.c

process.o:	process.c include/process.h
	$(CROSS)$(CC) $(CFLAGS) $(INCLUDE) -o process.o -c process.c

scheduler.o:	scheduler.c include/scheduler.h
	$(CROSS)$(CC) $(CFLAGS) $(INCLUDE) -o scheduler.o -c scheduler.c

time.o:	time.c include/time.h
	$(CROSS)$(CC) $(CFLAGS) $(INCLUDE) -o time.o -c time.c

wait.o:	wait.c include/wait.h
	$(CROSS)$(CC) $(CFLAGS) $(INCLUDE) -o wait.o -c wait.c

syscalls.o:	syscalls.c
	$(CROSS)$(CC) $(CFLAGS) $(INCLUDE) -o syscalls.o -c syscalls.c

swi_handler.o:	swi_handler.s
	$(CROSS)as $(ASFLAGS) -o swi_handler.o swi_handler.s

locks.o:	locks.s
	$(CROSS)as $(ASFLAGS) -o locks.o locks.s


### ARM1176

boot/boot-arm1176.a:	force_look
	cd boot && make

debug/debug-arm1176.a:	force_look
	cd debug && make

drivers/drivers.arm1176.a:	force_look
	cd drivers && make

interrupts/interrupts-arm1176.a:	force_look
	cd interrupts && make

lib/lib-arm1176.a:	force_look
	cd lib && make

memory/memory-arm1176.a:	force_look
	cd memory && make

### ARMV7

boot/boot-armv7.a:	force_look
	cd boot && make

debug/debug-armv7.a:	force_look
	cd debug && make

drivers/drivers.armv7.a:	force_look
	cd drivers && make

interrupts/interrupts-armv7.a:	force_look
	cd interrupts && make

lib/lib-armv7.a:	force_look
	cd lib && make

memory/memory-armv7.a:	force_look
	cd memory && make

###





fs/fs.a:	force_look
	cd fs && make

syscalls/syscalls.a:	force_look
	cd syscalls && make





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
	cd syscalls && make clean
	cd lib && make clean
