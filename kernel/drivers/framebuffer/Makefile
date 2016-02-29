include ../../../Makefile.inc
include ../../Makefile_kernel.inc

INCLUDE = -I../../include

all:	framebuffer.a

framebuffer.a:	framebuffer.o framebuffer_console.o
	$(CROSS)$(AR) crs framebuffer.a framebuffer.o framebuffer_console.o

framebuffer.o:	framebuffer.c
	$(CROSS)$(CC) $(CFLAGS) $(INCLUDE) -o framebuffer.o -c framebuffer.c

framebuffer_console.o:	framebuffer_console.c
	$(CROSS)$(CC) $(CFLAGS) $(INCLUDE) -o framebuffer_console.o -c framebuffer_console.c

clean:
	rm -f *~ *.o *.a



