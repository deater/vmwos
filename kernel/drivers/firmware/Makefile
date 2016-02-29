include ../../../Makefile.inc
include ../../Makefile_kernel.inc

INCLUDE = -I../../include

all:	firmware.a

firmware.a:	mailbox.o
	$(CROSS)$(AR) crs firmware.a mailbox.o

mailbox.o:	mailbox.c
	$(CROSS)$(CC) $(CFLAGS) $(INCLUDE) -o mailbox.o -c mailbox.c

clean:
	rm -f *~ *.o *.a



