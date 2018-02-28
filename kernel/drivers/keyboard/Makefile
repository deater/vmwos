include ../../../Makefile.inc
include ../../Makefile_kernel.inc

INCLUDE = -I../../include

all:	keyboard.arm1176.a keyboard.armv7.a

##### ARM1176

keyboard.arm1176.a:	ps2-keyboard.arm1176.o
	$(CROSS)$(AR) crs keyboard.arm1176.a ps2-keyboard.arm1176.o

ps2-keyboard.arm1176.o:	ps2-keyboard.c
	$(CROSS)$(CC) $(CFLAGS_ARM1176) $(INCLUDE) -o ps2-keyboard.arm1176.o -c ps2-keyboard.c

##### ARMV7

keyboard.armv7.a:	ps2-keyboard.armv7.o
	$(CROSS)$(AR) crs keyboard.armv7.a ps2-keyboard.armv7.o

ps2-keyboard.armv7.o:	ps2-keyboard.c
	$(CROSS)$(CC) $(CFLAGS_ARMV7) $(INCLUDE) -o ps2-keyboard.armv7.o -c ps2-keyboard.c

#####

clean:
	rm -f *~ *.o *.a



