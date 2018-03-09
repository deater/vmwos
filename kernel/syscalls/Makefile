include ../../Makefile.inc
include ../Makefile_kernel.inc

INCLUDE = -I../include

all:	syscalls-arm1176.a syscalls-armv7.a

### ARM1176

syscalls-arm1176.a:	syscalls.arm1176.o swi_handler.arm1176.o \
		exit.arm1176.o nanosleep.arm1176.o sysinfo.arm1176.o \
		times.arm1176.o uname.arm1176.o waitpid.arm1176.o
	$(CROSS)$(AR) crs syscalls-arm1176.a \
		syscalls.arm1176.o swi_handler.arm1176.o \
		exit.arm1176.o nanosleep.arm1176.o sysinfo.arm1176.o \
		times.arm1176.o uname.arm1176.o waitpid.arm1176.o


syscalls.arm1176.o:	syscalls.c
	$(CROSS)$(CC) $(CFLAGS_ARM1176) $(INCLUDE) -o syscalls.arm1176.o -c syscalls.c

swi_handler.arm1176.o:  swi_handler.s
	$(CROSS)as $(ASFLAGS_ARM1176) -o swi_handler.arm1176.o swi_handler.s

exit.arm1176.o:	exit.c ../include/syscalls/exit.h
	$(CROSS)$(CC) $(CFLAGS_ARM1176) $(INCLUDE) -o exit.arm1176.o -c exit.c

nanosleep.arm1176.o:	nanosleep.c ../include/syscalls/nanosleep.h
	$(CROSS)$(CC) $(CFLAGS_ARM1176) $(INCLUDE) -o nanosleep.arm1176.o -c nanosleep.c

sysinfo.arm1176.o:	sysinfo.c ../include/syscalls/sysinfo.h
	$(CROSS)$(CC) $(CFLAGS_ARM1176) $(INCLUDE) -o sysinfo.arm1176.o -c sysinfo.c

times.arm1176.o:	times.c ../include/syscalls/times.h
	$(CROSS)$(CC) $(CFLAGS_ARM1176) $(INCLUDE) -o times.arm1176.o -c times.c

uname.arm1176.o:	uname.c ../include/syscalls/uname.h ../date.h
	$(CROSS)$(CC) $(CFLAGS_ARM1176) $(INCLUDE) -o uname.arm1176.o -c uname.c

waitpid.arm1176.o:	waitpid.c ../include/syscalls/waitpid.h
	$(CROSS)$(CC) $(CFLAGS_ARM1176) $(INCLUDE) -o waitpid.arm1176.o -c waitpid.c


### ARMV7

syscalls-armv7.a:	syscalls.armv7.o swi_handler.armv7.o exit.armv7.o \
		nanosleep.armv7.o sysinfo.armv7.o times.armv7.o uname.armv7.o \
		waitpid.armv7.o
	$(CROSS)$(AR) crs syscalls-armv7.a \
			syscalls.armv7.o swi_handler.armv7.o exit.armv7.o \
		nanosleep.armv7.o sysinfo.armv7.o times.armv7.o uname.armv7.o \
		waitpid.armv7.o



syscalls.armv7.o:	syscalls.c
	$(CROSS)$(CC) $(CFLAGS_ARMV7) $(INCLUDE) -o syscalls.armv7.o -c syscalls.c

swi_handler.armv7.o:  swi_handler.s
	$(CROSS)as $(ASFLAGS_ARMV7) -o swi_handler.armv7.o swi_handler.s

exit.armv7.o:	exit.c ../include/syscalls/exit.h
	$(CROSS)$(CC) $(CFLAGS_ARMV7) $(INCLUDE) -o exit.armv7.o -c exit.c

nanosleep.armv7.o:	nanosleep.c ../include/syscalls/nanosleep.h
	$(CROSS)$(CC) $(CFLAGS_ARMV7) $(INCLUDE) -o nanosleep.armv7.o -c nanosleep.c

sysinfo.armv7.o:	sysinfo.c ../include/syscalls/sysinfo.h
	$(CROSS)$(CC) $(CFLAGS_ARMV7) $(INCLUDE) -o sysinfo.armv7.o -c sysinfo.c

times.armv7.o:	times.c ../include/syscalls/times.h
	$(CROSS)$(CC) $(CFLAGS_ARMV7) $(INCLUDE) -o times.armv7.o -c times.c

uname.armv7.o:	uname.c ../include/syscalls/uname.h ../date.h
	$(CROSS)$(CC) $(CFLAGS_ARMV7) $(INCLUDE) -o uname.armv7.o -c uname.c

waitpid.armv7.o:	waitpid.c ../include/syscalls/waitpid.h
	$(CROSS)$(CC) $(CFLAGS_ARMV7) $(INCLUDE) -o waitpid.armv7.o -c waitpid.c

###

clean:
	rm -f *~ *.o *.a ../date.h

force_look:
	true

../date.h:	force_look
	echo -n "#define COMPILE_DATE \"" > ../date.h
	date | tr -d '\n' >> ../date.h
	echo "\"" >> ../date.h

