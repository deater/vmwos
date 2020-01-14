include ../../Makefile.inc
include ../Makefile_kernel.inc

INCLUDE = -I../include

all:	processes-arm1176.a processes-armv7.a

### ARM1176

processes-arm1176.a:	process.arm1176.o exec.arm1176.o exit.arm1176.o \
		idle_task.arm1176.o scheduler.arm1176.o userspace.arm1176.o \
		vfork.arm1176.o waitpid.arm1176.o waitqueue.arm1176.o \
		bflt_loader.arm1176.o
	$(CROSS)$(AR) crs processes-arm1176.a \
			process.arm1176.o exec.arm1176.o exit.arm1176.o \
		idle_task.arm1176.o scheduler.arm1176.o userspace.arm1176.o \
		vfork.arm1176.o waitpid.arm1176.o waitqueue.arm1176.o \
		bflt_loader.arm1176.o


bflt_loader.arm1176.o:	bflt_loader.c
	$(CROSS)$(CC) $(CFLAGS_ARM1176) $(INCLUDE) -o bflt_loader.arm1176.o -c bflt_loader.c

exec.arm1176.o:	exec.c
	$(CROSS)$(CC) $(CFLAGS_ARM1176) $(INCLUDE) -o exec.arm1176.o -c exec.c

exit.arm1176.o:	exit.c
	$(CROSS)$(CC) $(CFLAGS_ARM1176) $(INCLUDE) -o exit.arm1176.o -c exit.c

idle_task.arm1176.o:	idle_task.c
	$(CROSS)$(CC) $(CFLAGS_ARM1176) $(INCLUDE) -o idle_task.arm1176.o -c idle_task.c

process.arm1176.o:	process.c
	$(CROSS)$(CC) $(CFLAGS_ARM1176) $(INCLUDE) -o process.arm1176.o -c process.c

scheduler.arm1176.o:	scheduler.c
	$(CROSS)$(CC) $(CFLAGS_ARM1176) $(INCLUDE) -o scheduler.arm1176.o -c scheduler.c

userspace.arm1176.o:	userspace.c
	$(CROSS)$(CC) $(CFLAGS_ARM1176) $(INCLUDE) -o userspace.arm1176.o -c userspace.c

vfork.arm1176.o:	vfork.c
	$(CROSS)$(CC) $(CFLAGS_ARM1176) $(INCLUDE) -o vfork.arm1176.o -c vfork.c

waitpid.arm1176.o:	waitpid.c
	$(CROSS)$(CC) $(CFLAGS_ARM1176) $(INCLUDE) -o waitpid.arm1176.o -c waitpid.c

waitqueue.arm1176.o:	waitqueue.c
	$(CROSS)$(CC) $(CFLAGS_ARM1176) $(INCLUDE) -o waitqueue.arm1176.o -c waitqueue.c

### ARMV7

processes-armv7.a:	process.armv7.o exec.armv7.o exit.armv7.o \
		idle_task.armv7.o scheduler.armv7.o userspace.armv7.o \
		vfork.armv7.o waitpid.armv7.o waitqueue.armv7.o \
		bflt_loader.armv7.o
	$(CROSS)$(AR) crs processes-armv7.a \
			process.armv7.o exec.armv7.o exit.armv7.o \
			idle_task.armv7.o scheduler.armv7.o userspace.armv7.o \
			vfork.armv7.o waitpid.armv7.o waitqueue.armv7.o \
			bflt_loader.armv7.o


bflt_loader.armv7.o:	bflt_loader.c
	$(CROSS)$(CC) $(CFLAGS_ARMV7) $(INCLUDE) -o bflt_loader.armv7.o -c bflt_loader.c

exec.armv7.o:	exec.c
	$(CROSS)$(CC) $(CFLAGS_ARMV7) $(INCLUDE) -o exec.armv7.o -c exec.c

exit.armv7.o:	exit.c
	$(CROSS)$(CC) $(CFLAGS_ARMV7) $(INCLUDE) -o exit.armv7.o -c exit.c

idle_task.armv7.o:	idle_task.c
	$(CROSS)$(CC) $(CFLAGS_ARMV7) $(INCLUDE) -o idle_task.armv7.o -c idle_task.c

process.armv7.o:	process.c
	$(CROSS)$(CC) $(CFLAGS_ARMV7) $(INCLUDE) -o process.armv7.o -c process.c

scheduler.armv7.o:	scheduler.c
	$(CROSS)$(CC) $(CFLAGS_ARMV7) $(INCLUDE) -o scheduler.armv7.o -c scheduler.c

userspace.armv7.o:	userspace.c
	$(CROSS)$(CC) $(CFLAGS_ARMV7) $(INCLUDE) -o userspace.armv7.o -c userspace.c

vfork.armv7.o:	vfork.c
	$(CROSS)$(CC) $(CFLAGS_ARM1176) $(INCLUDE) -o vfork.armv7.o -c vfork.c

waitpid.armv7.o:	waitpid.c
	$(CROSS)$(CC) $(CFLAGS_ARM1176) $(INCLUDE) -o waitpid.armv7.o -c waitpid.c

waitqueue.armv7.o:	waitqueue.c
	$(CROSS)$(CC) $(CFLAGS_ARM1176) $(INCLUDE) -o waitqueue.armv7.o -c waitqueue.c

###

clean:
	rm -f *~ *.o *.a
