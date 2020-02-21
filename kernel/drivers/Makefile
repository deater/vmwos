include ../../Makefile.inc
include ../Makefile_kernel.inc

INCLUDE = -I../include

all:	drivers-arm1176.a drivers-armv7.a

#### ARM1176

drivers-arm1176.a:	drivers.arm1176.o \
		block.arm1176.o \
		char.arm1176.o \
		audio/audio.arm1176.a \
		bcm2835/bcm2835.arm1176.a \
		ramdisk/ramdisk.arm1176.a \
		console/console.arm1176.a \
		firmware/firmware.arm1176.a \
		framebuffer/framebuffer.arm1176.a \
		gpio/gpio.arm1176.a \
		keyboard/keyboard.arm1176.a \
		led/led.arm1176.a \
		pmu/pmu.arm1176.a \
		random/random.arm1176.a \
		serial/serial.arm1176.a \
		thermal/thermal.arm1176.a \
		timer/timer.arm1176.a

	$(CROSS)$(AR) crsT drivers-arm1176.a \
		drivers.arm1176.o \
		ramdisk/ramdisk.arm1176.a \
		block.arm1176.o \
		char.arm1176.o \
		audio/audio.arm1176.a \
		bcm2835/bcm2835.arm1176.a \
		console/console.arm1176.a \
		firmware/firmware.arm1176.a \
		framebuffer/framebuffer.arm1176.a \
		gpio/gpio.arm1176.a \
		keyboard/keyboard.arm1176.a \
		led/led.arm1176.a \
		pmu/pmu.arm1176.a \
		random/random.arm1176.a \
		serial/serial.arm1176.a \
		thermal/thermal.arm1176.a \
		timer/timer.arm1176.a

drivers.arm1176.o:	drivers.c
	$(CROSS)$(CC) $(CFLAGS_ARM1176) $(INCLUDE) -o drivers.arm1176.o -c drivers.c

block.arm1176.o:	block.c
	$(CROSS)$(CC) $(CFLAGS_ARM1176) $(INCLUDE) -o block.arm1176.o -c block.c

char.arm1176.o:	char.c
	$(CROSS)$(CC) $(CFLAGS_ARM1176) $(INCLUDE) -o char.arm1176.o -c char.c


audio/audio.arm1176.a:	force_look
	cd audio && make

bcm2835/bcm2835.arm1176.a:	force_look
	cd bcm2835 && make

ramdisk/ramdisk.arm1176.a:	force_look
	cd ramdisk && make

console/console.arm1176.a:	force_look
	cd console && make

firmware/firmware.arm1176.a:	force_look
	cd firmware && make

framebuffer/framebuffer.arm1176.a:	force_look
	cd framebuffer && make

gpio/gpio.arm1176.a:	force_look
	cd gpio && make

keyboard/keyboard.arm1176.a:	force_look
	cd keyboard && make

led/led.arm1176.a:	force_look
	cd led && make

pmu/pmu.arm1176.a:	force_look
	cd pmu && make

random/random.arm1176.a:	force_look
	cd random && make

serial/serial.arm1176.a:	force_look
	cd serial && make

thermal/thermal.arm1176.a:	force_look
	cd thermal && make

timer/timer.arm1176.a:	force_look
	cd timer && make


#### ARMV7

drivers-armv7.a:	\
		drivers.armv7.o \
		ramdisk/ramdisk.armv7.a \
		block.armv7.o \
		char.armv7.o \
		audio/audio.armv7.a \
		bcm2835/bcm2835.armv7.a \
		console/console.armv7.a \
		firmware/firmware.armv7.a \
		framebuffer/framebuffer.armv7.a \
		gpio/gpio.armv7.a \
		keyboard/keyboard.armv7.a \
		led/led.armv7.a \
		pmu/pmu.armv7.a \
		random/random.armv7.a \
		serial/serial.armv7.a \
		thermal/thermal.armv7.a \
		timer/timer.armv7.a

	$(CROSS)$(AR) crsT drivers-armv7.a \
		drivers.armv7.o \
		block.armv7.o \
		char.armv7.o \
		audio/audio.armv7.a \
		bcm2835/bcm2835.armv7.a \
		ramdisk/ramdisk.armv7.a \
		console/console.armv7.a \
		firmware/firmware.armv7.a \
		framebuffer/framebuffer.armv7.a \
		gpio/gpio.armv7.a \
		keyboard/keyboard.armv7.a \
		led/led.armv7.a \
		pmu/pmu.armv7.a \
		random/random.armv7.a \
		serial/serial.armv7.a \
		thermal/thermal.armv7.a \
		timer/timer.armv7.a


drivers.armv7.o:	drivers.c
	$(CROSS)$(CC) $(CFLAGS_ARMV7) $(INCLUDE) -o drivers.armv7.o -c drivers.c

block.armv7.o:	block.c
	$(CROSS)$(CC) $(CFLAGS_ARMV7) $(INCLUDE) -o block.armv7.o -c block.c

char.armv7.o:	char.c
	$(CROSS)$(CC) $(CFLAGS_ARMV7) $(INCLUDE) -o char.armv7.o -c char.c

audio/audio.armv7.a:	force_look
	cd audio && make

bcm2835/bcm2835.armv7.a:	force_look
	cd bcm2835 && make

ramdisk/ramdisk.armv7.a:	force_look
	cd ramdisk && make

console/console.armv7.a:	force_look
	cd console && make

firmware/firmware.armv7.a:	force_look
	cd firmware && make

framebuffer/framebuffer.armv7.a:	force_look
	cd framebuffer && make

gpio/gpio.armv7.a:	force_look
	cd gpio && make

keyboard/keyboard.armv7.a:	force_look
	cd keyboard && make

led/led.armv7.a:	force_look
	cd led && make

pmu/pmu.armv7.a:	force_look
	cd pmu && make

random/random.armv7.a:	force_look
	cd random && make

serial/serial.armv7.a:	force_look
	cd serial && make

thermal/thermal.armv7.a:	force_look
	cd thermal && make

timer/timer.armv7.a:	force_look
	cd timer && make

###

force_look:
	true


clean:
	rm -f *~ *.o *.a
	cd audio && make clean
	cd bcm2835 && make clean
	cd console && make clean
	cd firmware && make clean
	cd framebuffer && make clean
	cd gpio && make clean
	cd keyboard && make clean
	cd led && make clean
	cd pmu && make clean
	cd ramdisk && make clean
	cd random && make clean
	cd serial && make clean
	cd thermal && make clean
	cd timer && make clean
