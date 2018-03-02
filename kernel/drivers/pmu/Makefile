include ../../../Makefile.inc
include ../../Makefile_kernel.inc

INCLUDE = -I../../include

all:	pmu.arm1176.a pmu.armv7.a

### ARM1176

pmu.arm1176.a:	arm1176-pmu.o
	$(CROSS)$(AR) crs pmu.arm1176.a arm1176-pmu.o

arm1176-pmu.o:	arm1176-pmu.c
	$(CROSS)$(CC) $(CFLAGS_ARM1176) $(INCLUDE) -o arm1176-pmu.o -c arm1176-pmu.c

### ARMV7

pmu.armv7.a:	armv7-pmu.o
	$(CROSS)$(AR) crs pmu.armv7.a armv7-pmu.o

armv7-pmu.o:	armv7-pmu.c
	$(CROSS)$(CC) $(CFLAGS_ARMV7) $(INCLUDE) -o armv7-pmu.o -c armv7-pmu.c

####

clean:
	rm -f *~ *.o *.a

