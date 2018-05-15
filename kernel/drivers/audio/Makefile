include ../../../Makefile.inc
include ../../Makefile_kernel.inc

INCLUDE = -I../../include

all:	audio.armv1176.a audio.armv7.a

#### ARM1176

audio.armv1176.a:	pwm_audio.arm1176.o
	$(CROSS)$(AR) crs audio.arm1176.a pwm_audio.arm1176.o


pwm_audio.arm1176.o:	pwm_audio.c
	$(CROSS)$(CC) $(CFLAGS_ARM1176) $(INCLUDE) -o pwm_audio.arm1176.o -c pwm_audio.c

#### ARMV7

audio.armv7.a:	pwm_audio.armv7.o
	$(CROSS)$(AR) crs audio.armv7.a pwm_audio.armv7.o

pwm_audio.armv7.o:	pwm_audio.c
	$(CROSS)$(CC) $(CFLAGS_ARMV7) $(INCLUDE) -o pwm_audio.armv7.o -c pwm_audio.c

clean:
	rm -f *~ *.o *.a



