include ../../Makefile.inc
include ../Makefile_user.inc

all:	vlibc.a

vlibc.a:	start.o \
		div.o error.o printf.o random.o stdio.o string.o system.o time.o tty.o \
		vmwos.o syscalls.arm.o
	$(CROSS)$(AR) crs vlibc.a start.o \
		div.o error.o printf.o random.o stdio.o string.o system.o time.o tty.o \
		vmwos.o syscalls.arm.o

start.o:	start.c
	$(CROSS)$(CC) $(CFLAGS) -c start.c

div.o:	div.c
	$(CROSS)$(CC) $(CFLAGS) -c div.c

error.o:	error.c
	$(CROSS)$(CC) $(CFLAGS) -c error.c

printf.o:	printf.c
	$(CROSS)$(CC) $(CFLAGS) -c printf.c

random.o:	random.c
	$(CROSS)$(CC) $(CFLAGS) -c random.c

stdio.o:	stdio.c
	$(CROSS)$(CC) $(CFLAGS) -c stdio.c

string.o:	string.c
	$(CROSS)$(CC) $(CFLAGS) -c string.c

system.o:	system.c
	$(CROSS)$(CC) $(CFLAGS) -c system.c

time.o:	time.c
	$(CROSS)$(CC) $(CFLAGS) -c time.c

tty.o:	tty.c
	$(CROSS)$(CC) $(CFLAGS) -c tty.c

vmwos.o:	vmwos.c
	$(CROSS)$(CC) $(CFLAGS) -c vmwos.c

syscalls.arm.o:	syscalls.arm.c
	$(CROSS)$(CC) $(CFLAGS) -c syscalls.arm.c

clean:
	rm -f *~ *.o *.a
