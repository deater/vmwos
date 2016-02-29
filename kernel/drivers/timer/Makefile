include ../../../Makefile.inc
include ../../Makefile_kernel.inc

INCLUDE = -I../../include

all:	timer.a

timer.a:	sp804_timer.o
	$(CROSS)$(AR) crs timer.a sp804_timer.o

sp804_timer.o:	sp804_timer.c
	$(CROSS)$(CC) $(CFLAGS) $(INCLUDE) -o sp804_timer.o -c sp804_timer.c

clean:
	rm -f *~ *.o *.a



