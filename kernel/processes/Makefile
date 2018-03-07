include ../../Makefile.inc
include ../Makefile_kernel.inc

INCLUDE = -I../include

all:	processes-arm1176.a processes-armv7.a

### ARM1176

processes-arm1176.a:	process.arm1176.o idle_task.arm1176.o scheduler.arm1176.o wait.arm1176.o
	$(CROSS)$(AR) crs processes-arm1176.a process.arm1176.o idle_task.arm1176.o scheduler.arm1176.o wait.arm1176.o


idle_task.arm1176.o:	idle_task.arm1176.c
	$(CROSS)$(CC) $(CFLAGS_ARM1176) $(INCLUDE) -o idle_task.arm1176.o -c idle_task.arm1176.c

process.arm1176.o:	process.c
	$(CROSS)$(CC) $(CFLAGS_ARM1176) $(INCLUDE) -o process.arm1176.o -c process.c

scheduler.arm1176.o:	scheduler.c
	$(CROSS)$(CC) $(CFLAGS_ARM1176) $(INCLUDE) -o scheduler.arm1176.o -c scheduler.c

wait.arm1176.o:	wait.c
	$(CROSS)$(CC) $(CFLAGS_ARM1176) $(INCLUDE) -o wait.arm1176.o -c wait.c

### ARMV7

processes-armv7.a:	process.armv7.o idle_task.armv7.o scheduler.armv7.o wait.armv7.o
	$(CROSS)$(AR) crs processes-armv7.a process.armv7.o idle_task.armv7.o scheduler.armv7.o wait.armv7.o


idle_task.armv7.o:	idle_task.armv7.c
	$(CROSS)$(CC) $(CFLAGS_ARMV7) $(INCLUDE) -o idle_task.armv7.o -c idle_task.armv7.c

process.armv7.o:	process.c
	$(CROSS)$(CC) $(CFLAGS_ARMV7) $(INCLUDE) -o process.armv7.o -c process.c

scheduler.armv7.o:	scheduler.c
	$(CROSS)$(CC) $(CFLAGS_ARMV7) $(INCLUDE) -o scheduler.armv7.o -c scheduler.c

wait.armv7.o:	wait.c
	$(CROSS)$(CC) $(CFLAGS_ARM1176) $(INCLUDE) -o wait.armv7.o -c wait.c

###

clean:
	rm -f *~ *.o *.a
