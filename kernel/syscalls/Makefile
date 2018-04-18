include ../../Makefile.inc
include ../Makefile_kernel.inc

INCLUDE = -I../include

all:	syscalls-arm1176.a syscalls-armv7.a

### ARM1176

syscalls-arm1176.a:	syscalls.arm1176.o swi_handler.arm1176.o \
		getcpu.arm1176.o \
		nanosleep.arm1176.o sysinfo.arm1176.o \
		times.arm1176.o uname.arm1176.o
	$(CROSS)$(AR) crs syscalls-arm1176.a \
		syscalls.arm1176.o swi_handler.arm1176.o \
		getcpu.arm1176.o \
		nanosleep.arm1176.o sysinfo.arm1176.o \
		times.arm1176.o uname.arm1176.o


syscalls.arm1176.o:	syscalls.c
	$(CROSS)$(CC) $(CFLAGS_ARM1176) $(INCLUDE) -o syscalls.arm1176.o -c syscalls.c

swi_handler.arm1176.o:  swi_handler.s
	$(CROSS)as $(ASFLAGS_ARM1176) -o swi_handler.arm1176.o swi_handler.s

getcpu.arm1176.o:	getcpu.c ../include/syscalls/getcpu.h
	$(CROSS)$(CC) $(CFLAGS_ARM1176) $(INCLUDE) -o getcpu.arm1176.o -c getcpu.c

nanosleep.arm1176.o:	nanosleep.c ../include/syscalls/nanosleep.h
	$(CROSS)$(CC) $(CFLAGS_ARM1176) $(INCLUDE) -o nanosleep.arm1176.o -c nanosleep.c

sysinfo.arm1176.o:	sysinfo.c ../include/syscalls/sysinfo.h
	$(CROSS)$(CC) $(CFLAGS_ARM1176) $(INCLUDE) -o sysinfo.arm1176.o -c sysinfo.c

times.arm1176.o:	times.c ../include/syscalls/times.h
	$(CROSS)$(CC) $(CFLAGS_ARM1176) $(INCLUDE) -o times.arm1176.o -c times.c

uname.arm1176.o:	uname.c ../include/syscalls/uname.h ../date.h
	$(CROSS)$(CC) $(CFLAGS_ARM1176) $(INCLUDE) -o uname.arm1176.o -c uname.c



### ARMV7

syscalls-armv7.a:	syscalls.armv7.o swi_handler.armv7.o \
		getcpu.armv7.o \
		nanosleep.armv7.o sysinfo.armv7.o times.armv7.o uname.armv7.o
	$(CROSS)$(AR) crs syscalls-armv7.a \
			syscalls.armv7.o swi_handler.armv7.o \
		getcpu.armv7.o \
		nanosleep.armv7.o sysinfo.armv7.o times.armv7.o uname.armv7.o



syscalls.armv7.o:	syscalls.c
	$(CROSS)$(CC) $(CFLAGS_ARMV7) $(INCLUDE) -o syscalls.armv7.o -c syscalls.c

swi_handler.armv7.o:  swi_handler.s
	$(CROSS)as $(ASFLAGS_ARMV7) -o swi_handler.armv7.o swi_handler.s

getcpu.armv7.o:	getcpu.c ../include/syscalls/getcpu.h
	$(CROSS)$(CC) $(CFLAGS_ARMV7) $(INCLUDE) -o getcpu.armv7.o -c getcpu.c

nanosleep.armv7.o:	nanosleep.c ../include/syscalls/nanosleep.h
	$(CROSS)$(CC) $(CFLAGS_ARMV7) $(INCLUDE) -o nanosleep.armv7.o -c nanosleep.c

sysinfo.armv7.o:	sysinfo.c ../include/syscalls/sysinfo.h
	$(CROSS)$(CC) $(CFLAGS_ARMV7) $(INCLUDE) -o sysinfo.armv7.o -c sysinfo.c

times.armv7.o:	times.c ../include/syscalls/times.h
	$(CROSS)$(CC) $(CFLAGS_ARMV7) $(INCLUDE) -o times.armv7.o -c times.c

uname.armv7.o:	uname.c ../include/syscalls/uname.h ../date.h
	$(CROSS)$(CC) $(CFLAGS_ARMV7) $(INCLUDE) -o uname.armv7.o -c uname.c

###

clean:
	rm -f *~ *.o *.a ../date.h

force_look:
	true

../date.h:	force_look
	/bin/echo -n "#define COMPILE_DATE \"" > ../date.h
	date | tr -d '\n' >> ../date.h
	/bin/echo "\"" >> ../date.h

