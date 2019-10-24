include ../../../Makefile.inc
include ../../Makefile_kernel.inc

INCLUDE = -I../../include

all:	framebuffer.arm1176.a framebuffer.armv7.a

#### ARM1176

framebuffer.arm1176.a:	framebuffer.arm1176.o framebuffer_console.arm1176.o
	$(CROSS)$(AR) crs framebuffer.arm1176.a framebuffer.arm1176.o framebuffer_console.arm1176.o

framebuffer.arm1176.o:	framebuffer.c
	$(CROSS)$(CC) $(CFLAGS_ARM1176) $(INCLUDE) -o framebuffer.arm1176.o -c framebuffer.c

framebuffer_console.arm1176.o:	framebuffer_console.c
	$(CROSS)$(CC) $(CFLAGS_ARM1176) $(INCLUDE) -o framebuffer_console.arm1176.o -c framebuffer_console.c

fast_memcpy.arm1176.o:	fast_memcpy.s
	$(CROSS)$(AS) $(ASFLAGS) -mfpu=vfp $(INCLUDE) -o fast_memcpy.arm1176.o fast_memcpy.s

#### ARMV7

framebuffer.armv7.a:	framebuffer.armv7.o framebuffer_console.armv7.o
	$(CROSS)$(AR) crs framebuffer.armv7.a framebuffer.armv7.o framebuffer_console.armv7.o

framebuffer.armv7.o:	framebuffer.c
	$(CROSS)$(CC) $(CFLAGS_ARMV7) $(INCLUDE) -o framebuffer.armv7.o -c framebuffer.c

framebuffer_console.armv7.o:	framebuffer_console.c
	$(CROSS)$(CC) $(CFLAGS_ARMV7) $(INCLUDE) -o framebuffer_console.armv7.o -c framebuffer_console.c

fast_memcpy.armv7.o:	fast_memcpy.s
	$(CROSS)$(AS) $(ASFLAGS_ARMV7) -mfpu=vfp $(INCLUDE) -o fast_memcpy.armv7.o fast_memcpy.s

####

clean:
	rm -f *~ *.o *.a



