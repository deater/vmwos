include ../../Makefile.inc
include ../Makefile_user.inc

all:	vlibc.a

vlibc.a:	start.o vmwos.o vlibc.o syscalls.arm.o
	$(CROSS)$(AR) crs vlibc.a start.o vmwos.o vlibc.o syscalls.arm.o

start.o:	start.c
	$(CROSS)$(CC) $(CFLAGS) -c start.c

vmwos.o:	vmwos.c
	$(CROSS)$(CC) $(CFLAGS) -c vmwos.c

vlibc.o:	vlibc.c
	$(CROSS)$(CC) $(CFLAGS) -c vlibc.c

syscalls.arm.o:	syscalls.arm.c
	$(CROSS)$(CC) $(CFLAGS) -c syscalls.arm.c

clean:
	rm -f *~ *.o *.a
