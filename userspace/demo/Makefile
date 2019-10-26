include ../../Makefile.inc

BUILDROOT = ../

include ../Makefile_user.inc

NATIVE_CFLAGS = -O2 -Wall -g
SDL_LIBS= `sdl-config --libs`
SDL_INCLUDE= `sdl-config --cflags`

all:	demosplash2019

native:	demosplash2019.native

####

sparkle:	sparkle.o svmwgraph.o pi-sim.o apple2.o
	$(CC) -o sparkle sparkle.o svmwgraph.o pi-sim.o apple2.o \
		$(SDL_LIBS) $(LFLAGS)

sparkle.o:	sparkle.c svmwgraph.h pi-sim.h
	$(CC) $(CFLAGS) -c sparkle.c

####

chiptune:	chiptune.o ay8912.o pt3_lib.o
	$(CC) -o chiptune ay8912.o chiptune.o pt3_lib.o $(LFLAGS)

####

chiptune.o:	chiptune.c pt3_lib.h
	$(CC) $(CFLAGS) -c chiptune.c

ay8912.o:	ay8912.c ayemu.h
	$(CC) $(CFLAGS) -c ay8912.c

pt3_lib.o:	pt3_lib.c pt3_lib.h
	$(CC) $(CFLAGS) -c pt3_lib.c

####

mode7_demo:	mode7_demo.o pi-sim.o svmwgraph.o apple2.o
	$(CC) -o mode7_demo mode7_demo.o svmwgraph.o apple2.o\
		pi-sim.o $(SDL_LIBS) $(LFLAGS)

mode7_demo.o:	mode7_demo.c svmwgraph.h pi-sim.h
	$(CC) $(CFLAGS) -c mode7_demo.c



####

demosplash2019.native:	demosplash2019.x86.o \
			pi-sim.x86.o svmwgraph.x86.o pcx_load.x86.o \
			vmw_open.x86.o print_string.x86.o \
			doom_fire.x86.o console.x86.o
	$(CC) -o demosplash2019.native demosplash2019.x86.o svmwgraph.x86.o \
		vmw_open.x86.o print_string.x86.o pcx_load.x86.o \
		doom_fire.x86.o console.x86.o \
		pi-sim.x86.o $(SDL_LIBS) $(NATIVE_LFLAGS)

demosplash2019:	demosplash2019.elf
	$(ELF2BFLT) demosplash2019.elf demosplash2019

demosplash2019.elf:	demosplash2019.o svmwgraph.o print_string.o \
		vmw_open.o pi-graphics.o doom_fire.o console.o \
		pcx_load.o \
		$(BUILDROOT)/vlibc/vlibc.a
	$(CROSS)$(CC) $(CFLAGS) -o demosplash2019.elf \
		demosplash2019.o svmwgraph.o vmw_open.o \
		pi-graphics.o print_string.o pcx_load.o \
		doom_fire.o console.o \
		$(BUILDROOT)/vlibc/vlibc.a $(LFLAGS)

demosplash2019.o:	demosplash2019.c demosplash2019.h svmwgraph.h
	$(CROSS)$(CC) $(CFLAGS) -c demosplash2019.c

demosplash2019.x86.o:	demosplash2019.c demosplash2019.h pi-graphics.h
	$(CC) $(NATIVE_CFLAGS) -o demosplash2019.x86.o -c demosplash2019.c

####

vmw_open.o:	vmw_open.c svmwgraph.h
	$(CROSS)$(CC) $(CFLAGS) -c vmw_open.c

vmw_open.x86.o:	vmw_open.c pi-graphics.h
	$(CC) $(NATIVE_CFLAGS) -o vmw_open.x86.o -c vmw_open.c


####

console.o:	console.c svmwgraph.h
	$(CROSS)$(CC) $(CFLAGS) -c console.c

console.x86.o:	console.c svmwgraph.h
	$(CC) $(NATIVE_CFLAGS) -o console.x86.o -c console.c

doom_fire.o:	doom_fire.c svmwgraph.h
	$(CROSS)$(CC) $(CFLAGS) -c doom_fire.c

doom_fire.x86.o:	doom_fire.c pi-graphics.h
	$(CC) $(NATIVE_CFLAGS) -o doom_fire.x86.o -c doom_fire.c



####

pi_opener:	pi_opener.o pcx_load.o \
		pi_boot.o pi-sim.o print_string.o svmwgraph.o
	$(CC) -o pi_opener pi_opener.o pcx_load.o \
		pi_boot.o print_string.o \
		pi-sim.o svmwgraph.o $(SDL_LIBS) $(LFLAGS)

pi_opener.o:	pi_opener.c pi-sim.h
	$(CC) $(CFLAGS) -c pi_opener.c


####

pi_boot.o:	pi_boot.pcx
	ld -r -b binary pi_boot.pcx -o pi_boot.o

####

#pi-sim.a:	pi-sim.o apple2.o lowlevel_graphics.o print_string.o default_font.o
#	$(AR) crs pi-sim.a pi-sim.o apple2.o lowlevel_graphics.o print_string.o default_font.o


###

default_font.o:	default_font.c default_font.h
	$(CROSS)$(CC) $(CFLAGS) -c default_font.c

default_font.x86.o:	default_font.c default_font.h
	$(CC) $(CFLAGS) -c default_font.c

pi-sim.x86.o:	pi-sim.c pi-graphics.h
	$(CC) $(NATIVE_CFLAGS) $(SDL_INCLUDE) -o pi-sim.x86.o -c pi-sim.c

pi-graphics.o:	pi-graphics.c pi-graphics.h
	$(CROSS)$(CC) $(CFLAGS) $(SDL_INCLUDE) -c pi-graphics.c

svmwgraph.o:	svmwgraph.c svmwgraph.h
	$(CROSS)$(CC) $(CFLAGS) $(SDL_INCLUDE) -c svmwgraph.c

svmwgraph.x86.o:	svmwgraph.c svmwgraph.h
	$(CC) $(NATIVE_CFLAGS) $(SDL_INCLUDE) -o svmwgraph.x86.o -c svmwgraph.c

apple2.o:	apple2.c pi-sim.h
	$(CC) $(CFLAGS) $(SDL_INCLUDE) -c apple2.c

pcx_load.o:	pcx_load.c svmwgraph.h
	$(CROSS)$(CC) $(CFLAGS) -c pcx_load.c

pcx_load.x86.o:	pcx_load.c svmwgraph.h
	$(CC) $(NATIVE_CFLAGS) -o pcx_load.x86.o -c pcx_load.c

print_string.o:	print_string.c default_font.h
	$(CROSS)$(CC) $(CFLAGS) -c print_string.c

print_string.x86.o:	print_string.c default_font.h
	$(CC) $(NATIVE_CFLAGS) -o print_string.x86.o -c print_string.c



clean:
	rm -f chiptune sparkle pi_opener vmw_open mode7_demo \
		*~ *.o *.a *.elf *.native

