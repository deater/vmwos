include ../../Makefile.inc
include ../Makefile_kernel.inc

INCLUDE = -I../include

# GCC 12 added this, annoys you to remembering that things like FP
#	not saved by the interrupt attribute
EXTRA_CFLAGS = -mgeneral-regs-only

all:	interrupts-arm1176.a interrupts-armv7.a

### ARM1176

interrupts-arm1176.a:	gic-400.arm1176.o interrupts.arm1176.o ipi.arm1176.o irq_handler.arm1176.o
	$(CROSS)$(AR) crs interrupts-arm1176.a gic-400.arm1176.o interrupts.arm1176.o ipi.arm1176.o irq_handler.arm1176.o

gic-400.arm1176.o:	gic-400.c
	$(CROSS)$(CC) $(CFLAGS_ARM1176) $(EXTRA_CFLAGS) $(INCLUDE) -o gic-400.arm1176.o -c gic-400.c

interrupts.arm1176.o:	interrupts.c
	$(CROSS)$(CC) $(CFLAGS_ARM1176) $(EXTRA_CFLAGS) $(INCLUDE) -o interrupts.arm1176.o -c interrupts.c

ipi.arm1176.o:	ipi.arm1176.c
	$(CROSS)$(CC) $(CFLAGS_ARM1176) $(INCLUDE) -o ipi.arm1176.o -c ipi.arm1176.c

irq_handler.arm1176.o:	irq_handler.s
	$(CROSS)as $(ASFLAGS_ARM1176) -o irq_handler.arm1176.o irq_handler.s

### ARMV7

interrupts-armv7.a:	gic-400.armv7.o interrupts.armv7.o ipi.armv7.o irq_handler.armv7.o
	$(CROSS)$(AR) crs interrupts-armv7.a gic-400.armv7.o interrupts.armv7.o ipi.armv7.o irq_handler.armv7.o

gic-400.armv7.o:	gic-400.c
	$(CROSS)$(CC) $(CFLAGS_ARMV7) $(EXTRA_CFLAGS) $(INCLUDE) -o gic-400.armv7.o -c gic-400.c

interrupts.armv7.o:	interrupts.c
	$(CROSS)$(CC) $(CFLAGS_ARMV7) $(EXTRA_CFLAGS) $(INCLUDE) -o interrupts.armv7.o -c interrupts.c

ipi.armv7.o:	ipi.armv7.c
	$(CROSS)$(CC) $(CFLAGS_ARMV7) $(INCLUDE) -o ipi.armv7.o -c ipi.armv7.c

irq_handler.armv7.o:	irq_handler.s
	$(CROSS)as $(ASFLAGS_ARMV7) -o irq_handler.armv7.o irq_handler.s

###

clean:
	rm -f *~ *.o *.a
