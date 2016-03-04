include ../Makefile.inc
include Makefile_kernel.inc

LINKER_SCRIPT = kernel.ld

INCLUDE = -I./include

all:	kernel.img kernel.dis

kernel.img:	kernel.elf
	$(CROSS)objcopy kernel.elf -O binary kernel.img



kernel.elf:	kernel_main.o \
	exec.o idle_task.o mmu.o syscalls.o \
	interrupts.o memory.o scheduler.o time.o \
	swi_handler.o irq_handler.o \
	boot/boot.a \
	drivers/drivers.a \
	fs/fs.a \
	lib/lib.a
	$(CROSS)ld $(LFLAGS) \
		kernel_main.o \
		exec.o idle_task.o mmu.o syscalls.o \
		interrupts.o memory.o scheduler.o time.o \
		swi_handler.o irq_handler.o \
		boot/boot.a \
		drivers/drivers.a \
		fs/fs.a \
		lib/lib.a \
		-Map kernel.map -o kernel.elf

kernel_main.o:	kernel_main.c
	$(CROSS)$(CC) $(CFLAGS) $(INCLUDE) -o kernel_main.o -c kernel_main.c

exec.o:	exec.c
	$(CROSS)$(CC) $(CFLAGS) $(INCLUDE) -o exec.o -c exec.c

idle_task.o:	idle_task.c
	$(CROSS)$(CC) $(CFLAGS) -o idle_task.o -c idle_task.c

interrupts.o:	interrupts.c
	$(CROSS)$(CC) $(CFLAGS) $(INCLUDE) -o interrupts.o -c interrupts.c

memory.o:	memory.c
	$(CROSS)$(CC) $(CFLAGS) $(INCLUDE) -o memory.o -c memory.c

mmu.o:	mmu.c
	$(CROSS)$(CC) $(CFLAGS) $(INCLUDE) -o mmu.o -c mmu.c

scheduler.o:	scheduler.c scheduler.h
	$(CROSS)$(CC) $(CFLAGS) $(INCLUDE) -o scheduler.o -c scheduler.c

syscalls.o:	syscalls.c
	$(CROSS)$(CC) $(CFLAGS) $(INCLUDE) -o syscalls.o -c syscalls.c

time.o:	time.c include/time.h
	$(CROSS)$(CC) $(CFLAGS) $(INCLUDE) -o time.o -c time.c

irq_handler.o:	irq_handler.s
	$(CROSS)as $(ASFLAGS) -o irq_handler.o irq_handler.s

swi_handler.o:	swi_handler.s
	$(CROSS)as $(ASFLAGS) -o swi_handler.o swi_handler.s

locks.o:	locks.s
	$(CROSS)as $(ASFLAGS) -o locks.o locks.s



boot/boot.a:	force_look
	cd boot && make

drivers/drivers.a:	force_look
	cd drivers && make

fs/fs.a:	force_look
	cd fs && make

lib/lib.a:	force_look
	cd lib && make



force_look:
		true


kernel.dis:	kernel.elf
	$(CROSS)objdump --disassemble-all kernel.elf > kernel.dis

clean:
	rm -f *~ *.o *.map *.elf kernel.img *.dis
	cd boot && make clean
	cd drivers && make clean
	cd fs && make clean
	cd lib && make clean
