include ../../Makefile.inc
include ../Makefile_kernel.inc

INCLUDE = -I../include

all:	memory-arm1176.a memory-armv7.a

### ARM1176

memory-arm1176.a:	arm1176-mmu.o memory.arm1176.o
	$(CROSS)$(AR) crs memory-arm1176.a arm1176-mmu.o memory.arm1176.o

arm1176-mmu.o:	arm1176-mmu.c
	$(CROSS)$(CC) $(CFLAGS_ARM1176) $(INCLUDE) -o arm1176-mmu.o -c arm1176-mmu.c

memory.arm1176.o:	memory.c
	$(CROSS)$(CC) $(CFLAGS_ARM1176) $(INCLUDE) -o memory.arm1176.o -c memory.c

### ARMV7

memory-armv7.a:	memory.armv7.o armv7-mmu.o
	$(CROSS)$(AR) crs memory-armv7.a memory.armv7.o armv7-mmu.o

armv7-mmu.o:	armv7-mmu.c
	$(CROSS)$(CC) $(CFLAGS_ARMV7) $(INCLUDE) -o armv7-mmu.o -c armv7-mmu.c

memory.armv7.o:	memory.c
	$(CROSS)$(CC) $(CFLAGS_ARMV7) $(INCLUDE) -o memory.armv7.o -c memory.c

###

clean:
	rm -f *~ *.o *.a
