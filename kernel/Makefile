include ../Makefile.inc
include Makefile_kernel.inc

LINKER_SCRIPT = kernel.ld

INCLUDE = -I./include

all:	kernel7.img kernel.img kernel.dis

kernel.img:	kernel.elf
	$(CROSS)objcopy kernel.elf -O binary kernel.img

kernel7.img:	kernel7.elf
	$(CROSS)objcopy kernel7.elf -O binary kernel7.img

kernel.elf:	kernel_main.o \
	idle_task.o arm1176-mmu.o arm1176-pmu.o syscalls.o \
	interrupts.o memory.o process.o scheduler.o time.o wait.o \
	swi_handler.o irq_handler.o \
	boot/boot-arm1176.a \
	debug/debug.a \
	drivers/drivers.a \
	fs/fs.a \
	syscalls/syscalls.a \
	lib/lib.a
	$(CROSS)ld $(LFLAGS) \
		kernel_main.o \
		idle_task.o \
		arm1176-mmu.o arm1176-pmu.o \
		syscalls.o \
		interrupts.o memory.o process.o scheduler.o time.o wait.o \
		swi_handler.o irq_handler.o \
		boot/boot-arm1176.a \
		drivers/drivers.a \
		debug/debug.a \
		fs/fs.a \
		syscalls/syscalls.a \
		lib/lib.a \
		-Map kernel.map -o kernel.elf

kernel7.elf:	kernel_main.o \
	idle_task.o arm1176-mmu.o arm1176-pmu.o syscalls.o \
	interrupts.o memory.o process.o scheduler.o time.o wait.o \
	swi_handler.o irq_handler.o \
	boot/boot-arm7.a \
	debug/debug.a \
	drivers/drivers.a \
	fs/fs.a \
	syscalls/syscalls.a \
	lib/lib.a
	$(CROSS)ld $(LFLAGS) \
		kernel_main.o \
		idle_task.o \
		arm1176-mmu.o arm1176-pmu.o \
		syscalls.o interrupts.o memory.o \
		process.o scheduler.o time.o wait.o \
		swi_handler.o irq_handler.o \
		boot/boot-arm7.a \
		drivers/drivers.a \
		debug/debug.a \
		fs/fs.a \
		syscalls/syscalls.a \
		lib/lib.a \
		-Map kernel7.map -o kernel7.elf

kernel_main.o:	kernel_main.c version.h
	$(CROSS)$(CC) $(CFLAGS) $(INCLUDE) -o kernel_main.o -c kernel_main.c

idle_task.o:	idle_task.c
	$(CROSS)$(CC) $(CFLAGS) -o idle_task.o -c idle_task.c

interrupts.o:	interrupts.c
	$(CROSS)$(CC) $(CFLAGS) $(INCLUDE) -o interrupts.o -c interrupts.c

memory.o:	memory.c
	$(CROSS)$(CC) $(CFLAGS) $(INCLUDE) -o memory.o -c memory.c

arm1176-mmu.o:	arm1176-mmu.c
	$(CROSS)$(CC) $(CFLAGS) $(INCLUDE) -o arm1176-mmu.o -c arm1176-mmu.c

arm1176-pmu.o:	arm1176-pmu.c
	$(CROSS)$(CC) $(CFLAGS) $(INCLUDE) -o arm1176-pmu.o -c arm1176-pmu.c

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

irq_handler.o:	irq_handler.s
	$(CROSS)as $(ASFLAGS) -o irq_handler.o irq_handler.s

swi_handler.o:	swi_handler.s
	$(CROSS)as $(ASFLAGS) -o swi_handler.o swi_handler.s

locks.o:	locks.s
	$(CROSS)as $(ASFLAGS) -o locks.o locks.s



boot/boot-arm1176.a:	force_look
	cd boot && make

boot/boot-arm7.a:	force_look
	cd boot && make

debug/debug.a:	force_look
	cd debug && make

drivers/drivers.a:	force_look
	cd drivers && make

fs/fs.a:	force_look
	cd fs && make

syscalls/syscalls.a:	force_look
	cd syscalls && make

lib/lib.a:	force_look
	cd lib && make



force_look:
		true

kernel.dis:	kernel.elf
	$(CROSS)objdump --disassemble-all kernel.elf > kernel.dis

clean:
	rm -f *~ *.o *.map *.elf kernel.img *.dis date.h
	cd boot && make clean
	cd debug && make clean
	cd drivers && make clean
	cd fs && make clean
	cd syscalls && make clean
	cd lib && make clean
