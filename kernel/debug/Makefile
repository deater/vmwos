include ../../Makefile.inc
include ../Makefile_kernel.inc

INCLUDE = -I../include

all:	debug-arm1176.a debug-armv7.a

### ARM1176

debug-arm1176.a:	panic.arm1176.o early_debug.arm1176.o emergency_blink.arm1176.o
	$(CROSS)$(AR) crs debug-arm1176.a panic.arm1176.o early_debug.arm1176.o emergency_blink.arm1176.o

panic.arm1176.o:	panic.c
	$(CROSS)$(CC) $(CFLAGS_ARM1176) $(INCLUDE) -o panic.arm1176.o -c panic.c

early_debug.arm1176.o:	early_debug.c
	$(CROSS)$(CC) $(CFLAGS_ARM1176) $(INCLUDE) -o early_debug.arm1176.o -c early_debug.c

emergency_blink.arm1176.o:	emergency_blink.c
	$(CROSS)$(CC) $(CFLAGS_ARM1176) $(INCLUDE) -o emergency_blink.arm1176.o -c emergency_blink.c

### ARMV7

debug-armv7.a:	panic.armv7.o early_debug.armv7.o emergency_blink.armv7.o
	$(CROSS)$(AR) crs debug-armv7.a panic.armv7.o early_debug.armv7.o emergency_blink.armv7.o

panic.armv7.o:	panic.c
	$(CROSS)$(CC) $(CFLAGS_ARMV7) $(INCLUDE) -o panic.armv7.o -c panic.c

early_debug.armv7.o:	early_debug.c
	$(CROSS)$(CC) $(CFLAGS_ARMV7) $(INCLUDE) -o early_debug.armv7.o -c early_debug.c

emergency_blink.armv7.o:	emergency_blink.c
	$(CROSS)$(CC) $(CFLAGS_ARMV7) $(INCLUDE) -o emergency_blink.armv7.o -c emergency_blink.c


clean:
	rm -f *~ *.o *.a
