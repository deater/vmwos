include ../../Makefile.inc
include ../Makefile_kernel.inc

INCLUDE = -I../include

all:	time-arm1176.a time-armv7.a

### ARM1176

time-arm1176.a:	time.arm1176.o
	$(CROSS)$(AR) crs time-arm1176.a time.arm1176.o

time.arm1176.o:	time.c
	$(CROSS)$(CC) $(CFLAGS_ARM1176) $(INCLUDE) -o time.arm1176.o -c time.c

### ARMV7

time-armv7.a:	time.armv7.o
	$(CROSS)$(AR) crs time-armv7.a time.armv7.o

time.armv7.o:	time.c
	$(CROSS)$(CC) $(CFLAGS_ARMV7) $(INCLUDE) -o time.armv7.o -c time.c

###

clean:
	rm -f *~ *.o *.a
