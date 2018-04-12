include ../../Makefile.inc
include ../Makefile_kernel.inc

INCLUDE = -I../include

all:	boot-armv7.a boot-arm1176.a

#### ARM1176

boot-arm1176.a:	atags.arm1176.o boot-arm1176.o device_tree.arm1176.o hardware_detect.arm1176.o
	$(CROSS)$(AR) crs boot-arm1176.a atags.arm1176.o boot-arm1176.o device_tree.arm1176.o hardware_detect.arm1176.o

atags.arm1176.o:	atags.c
	$(CROSS)$(CC) $(CFLAGS_ARM1176) $(INCLUDE) -o atags.arm1176.o -c atags.c

boot-arm1176.o:	boot-arm1176.s
	$(CROSS)as $(ASFLAGS_ARM1176) $(INCLUDE) -o boot-arm1176.o boot-arm1176.s

device_tree.arm1176.o:	device_tree.c
	$(CROSS)$(CC) $(CFLAGS_ARM1176) $(INCLUDE) -o device_tree.arm1176.o -c device_tree.c

hardware_detect.arm1176.o:	hardware_detect.c
	$(CROSS)$(CC) $(CFLAGS_ARM1176) $(INCLUDE) -o hardware_detect.arm1176.o -c hardware_detect.c

#### AMV7

boot-armv7.a:	atags.armv7.o boot-armv7.o device_tree.armv7.o \
		hardware_detect.armv7.o smp_boot.armv7.o \
		start_smp-armv7.o
	$(CROSS)$(AR) crs boot-armv7.a atags.armv7.o boot-armv7.o \
			device_tree.armv7.o hardware_detect.armv7.o \
			smp_boot.armv7.o start_smp-armv7.o

atags.armv7.o:	atags.c
	$(CROSS)$(CC) $(CFLAGS_ARMV7) $(INCLUDE) -o atags.armv7.o -c atags.c

boot-armv7.o:	boot-armv7.s
	$(CROSS)as $(ASFLAGS_ARMV7) $(INCLUDE) -o boot-armv7.o boot-armv7.s

device_tree.armv7.o:	device_tree.c
	$(CROSS)$(CC) $(CFLAGS_ARMV7) $(INCLUDE) -o device_tree.armv7.o -c device_tree.c

hardware_detect.armv7.o:	hardware_detect.c
	$(CROSS)$(CC) $(CFLAGS_ARMV7) $(INCLUDE) -o hardware_detect.armv7.o -c hardware_detect.c

smp_boot.armv7.o:	smp_boot.c
	$(CROSS)$(CC) $(CFLAGS_ARMV7) $(INCLUDE) -o smp_boot.armv7.o -c smp_boot.c

start_smp-armv7.o:	start_smp-armv7.s
	$(CROSS)as $(ASFLAGS_ARMV7) $(INCLUDE) -o start_smp-armv7.o start_smp-armv7.s

###

dt_standalone:	dt_standalone.o
	gcc -o dt_standalone dt_standalone.o

dt_standalone.o:	device_tree.c
	gcc -Wall -O2 -DSTANDALONE_TEST=1 -o dt_standalone.o -c device_tree.c


###

clean:
	rm -f *~ *.o *.a dt_standalone
