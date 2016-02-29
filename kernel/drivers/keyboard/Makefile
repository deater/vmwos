include ../../../Makefile.inc
include ../../Makefile_kernel.inc

INCLUDE = -I../../include

all:	keyboard.a

keyboard.a:	ps2-keyboard.o
	$(CROSS)$(AR) crs keyboard.a ps2-keyboard.o

ps2-keyboard.o:	ps2-keyboard.c
	$(CROSS)$(CC) $(CFLAGS) $(INCLUDE) -o ps2-keyboard.o -c ps2-keyboard.c


clean:
	rm -f *~ *.o *.a



