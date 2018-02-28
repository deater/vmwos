include ../../../Makefile.inc
include ../../Makefile_kernel.inc

INCLUDE = -I../../include

all:	firmware.arm1176.a firmware.armv7.a

#### ARM1176

firmware.arm1176.a:	mailbox.arm1176.o
	$(CROSS)$(AR) crs firmware.arm1176.a mailbox.arm1176.o

mailbox.arm1176.o:	mailbox.c
	$(CROSS)$(CC) $(CFLAGS_ARM1176) $(INCLUDE) -o mailbox.arm1176.o -c mailbox.c

#### ARMV7

firmware.armv7.a:	mailbox.armv7.o
	$(CROSS)$(AR) crs firmware.armv7.a mailbox.armv7.o

mailbox.armv7.o:	mailbox.c
	$(CROSS)$(CC) $(CFLAGS_ARMV7) $(INCLUDE) -o mailbox.armv7.o -c mailbox.c

####

clean:
	rm -f *~ *.o *.a



