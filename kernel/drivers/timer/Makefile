include ../../../Makefile.inc
include ../../Makefile_kernel.inc

INCLUDE = -I../../include

all:	timer.arm1176.a timer.armv7.a

#### ARM1176

timer.arm1176.a:	sp804_timer.arm1176.o
	$(CROSS)$(AR) crs timer.arm1176.a sp804_timer.arm1176.o

sp804_timer.arm1176.o:	sp804_timer.c
	$(CROSS)$(CC) $(CFLAGS_ARM1176) $(INCLUDE) -o sp804_timer.arm1176.o -c sp804_timer.c

#### ARMV7

timer.armv7.a:	sp804_timer.armv7.o
	$(CROSS)$(AR) crs timer.armv7.a sp804_timer.armv7.o

sp804_timer.armv7.o:	sp804_timer.c
	$(CROSS)$(CC) $(CFLAGS_ARMV7) $(INCLUDE) -o sp804_timer.armv7.o -c sp804_timer.c

####

clean:
	rm -f *~ *.o *.a



