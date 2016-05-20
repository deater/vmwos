include ../../Makefile.inc
include ../Makefile_kernel.inc

INCLUDE = -I../include

all:	syscalls.a

syscalls.a:	exec.o exit.o nanosleep.o sysinfo.o times.o uname.o vfork.o wait.o
	$(CROSS)$(AR) crs syscalls.a exec.o exit.o nanosleep.o sysinfo.o times.o uname.o vfork.o wait.o

exec.o:	exec.c ../include/syscalls/exec.h
	$(CROSS)$(CC) $(CFLAGS) $(INCLUDE) -o exec.o -c exec.c

exit.o:	exit.c ../include/syscalls/exit.h
	$(CROSS)$(CC) $(CFLAGS) $(INCLUDE) -o exit.o -c exit.c

nanosleep.o:	nanosleep.c ../include/syscalls/nanosleep.h
	$(CROSS)$(CC) $(CFLAGS) $(INCLUDE) -o nanosleep.o -c nanosleep.c

sysinfo.o:	sysinfo.c ../include/syscalls/sysinfo.h
	$(CROSS)$(CC) $(CFLAGS) $(INCLUDE) -o sysinfo.o -c sysinfo.c

times.o:	times.c ../include/syscalls/times.h
	$(CROSS)$(CC) $(CFLAGS) $(INCLUDE) -o times.o -c times.c

uname.o:	uname.c ../include/syscalls/uname.h date.h
	$(CROSS)$(CC) $(CFLAGS) $(INCLUDE) -o uname.o -c uname.c

vfork.o:	vfork.c ../include/syscalls/vfork.h
	$(CROSS)$(CC) $(CFLAGS) $(INCLUDE) -o vfork.o -c vfork.c

wait.o:	wait.c ../include/syscalls/wait.h
	$(CROSS)$(CC) $(CFLAGS) $(INCLUDE) -o wait.o -c wait.c

clean:
	rm -f *~ *.o *.a date.h

force_look:
	true

date.h:	force_look
	echo -n "#define COMPILE_DATE \"" > date.h
	date | tr -d '\n' >> date.h
	echo "\"" >> date.h

