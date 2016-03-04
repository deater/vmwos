include ../../Makefile.inc
include ../Makefile_kernel.inc

all:	drivers.a

drivers.a:	\
		block/block.a \
		console/console.a \
		firmware/firmware.a \
		framebuffer/framebuffer.a \
		gpio/gpio.a \
		keyboard/keyboard.a \
		led/led.a \
		serial/serial.a \
		thermal/thermal.a \
		timer/timer.a

	$(CROSS)$(AR) crsT drivers.a \
		block/block.a \
		console/console.a \
		firmware/firmware.a \
		framebuffer/framebuffer.a \
		gpio/gpio.a \
		keyboard/keyboard.a \
		led/led.a \
		serial/serial.a \
		thermal/thermal.a \
		timer/timer.a

block/block.a:	force_look
	cd block && make

console/console.a:	force_look
	cd console && make

firmware/firmware.a:	force_look
	cd firmware && make

framebuffer/framebuffer.a:	force_look
	cd framebuffer && make

gpio/gpio.a:	force_look
	cd gpio && make

keyboard/keyboard.a:	force_look
	cd keyboard && make

led/led.a:	force_look
	cd led && make

serial/serial.a:	force_look
	cd serial && make

thermal/thermal.a:	force_look
	cd thermal && make

timer/timer.a:	force_look
	cd timer && make

force_look:
	true


clean:
	rm -f *~ *.o *.a
	cd block && make clean
	cd console && make clean
	cd firmware && make clean
	cd framebuffer && make clean
	cd gpio && make clean
	cd keyboard && make clean
	cd led && make clean
	cd serial && make clean
	cd thermal && make clean
	cd timer && make clean



