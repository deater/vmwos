include ../../Makefile.inc

BUILDROOT = ../

include ../Makefile_user.inc

NATIVE_CFLAGS = -O2 -Wall -g
SDL_LIBS= `sdl-config --libs`
SDL_INCLUDE= `sdl-config --cflags`

all:	demosplash2019

native:	demosplash2019.native

####

demosplash2019.native:	demosplash2019.x86.o \
			pi-sim.x86.o svmwgraph.x86.o pcx_load.x86.o \
			vmw_open.x86.o print_string.x86.o mode7.x86.o \
			doom_fire.x86.o console.x86.o boot_intro.x86.o \
			ay8912.x86.o pt3_lib.x86.o chiptune.x86.o
	$(CC) -o demosplash2019.native demosplash2019.x86.o svmwgraph.x86.o \
		vmw_open.x86.o print_string.x86.o pcx_load.x86.o \
		doom_fire.x86.o console.x86.o boot_intro.x86.o \
		mode7.x86.o ay8912.x86.o pt3_lib.x86.o chiptune.x86.o \
		pi-sim.x86.o $(SDL_LIBS) $(NATIVE_LFLAGS)

demosplash2019:	demosplash2019.elf
	$(ELF2BFLT) demosplash2019.elf demosplash2019

demosplash2019.elf:	demosplash2019.o svmwgraph.o print_string.o \
		vmw_open.o pi-graphics.o doom_fire.o console.o \
		pcx_load.o boot_intro.o mode7.o \
		ay8912.o pt3_lib.o chiptune.o \
		$(BUILDROOT)/vlibc/vlibc.a
	$(CROSS)$(CC) $(CFLAGS) -o demosplash2019.elf \
		demosplash2019.o svmwgraph.o vmw_open.o \
		pi-graphics.o print_string.o pcx_load.o \
		doom_fire.o console.o boot_intro.o mode7.o \
		ay8912.o pt3_lib.o chiptune.o \
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

ay8912.o:	ay8912.c ayemu.h
	$(CROSS)$(CC) $(CFLAGS) -c ay8912.c

ay8912.x86.o:	ay8912.c ayemu.h
	$(CC) $(NATIVE_CFLAGS) -o ay8912.x86.o -c ay8912.c

chiptune.o:	chiptune.c pt3_lib.h
	$(CROSS)$(CC) $(CFLAGS) -c chiptune.c

chiptune.x86.o:	chiptune.c pt3_lib.h
	$(CC) $(NATIVE_CFLAGS) -o chiptune.x86.o -c chiptune.c

pt3_lib.o:	pt3_lib.c pt3_lib.h
	$(CROSS)$(CC) $(CFLAGS) -c pt3_lib.c

pt3_lib.x86.o:	pt3_lib.c pt3_lib.h
	$(CC) $(NATIVE_CFLAGS) -o pt3_lib.x86.o -c pt3_lib.c

####

boot_intro.o:	boot_intro.c svmwgraph.h pi1_dmesg.h
	$(CROSS)$(CC) $(CFLAGS) -c boot_intro.c

boot_intro.x86.o:	boot_intro.c svmwgraph.h pi1_dmesg.h
	$(CC) $(NATIVE_CFLAGS) -o boot_intro.x86.o -c boot_intro.c

console.o:	console.c svmwgraph.h
	$(CROSS)$(CC) $(CFLAGS) -c console.c

console.x86.o:	console.c svmwgraph.h
	$(CC) $(NATIVE_CFLAGS) -o console.x86.o -c console.c

doom_fire.o:	doom_fire.c svmwgraph.h
	$(CROSS)$(CC) $(CFLAGS) -c doom_fire.c

doom_fire.x86.o:	doom_fire.c pi-graphics.h
	$(CC) $(NATIVE_CFLAGS) -o doom_fire.x86.o -c doom_fire.c

mode7.o:	mode7.c svmwgraph.h pi_actual.h flight_replay.h pi_ship.h deep_field.h
	$(CROSS)$(CC) $(CFLAGS) -c mode7.c

mode7.x86.o:	mode7.c pi-graphics.h pi_actual.h flight_replay.h pi_ship.h deep_field.h
	$(CC) $(NATIVE_CFLAGS) -o mode7.x86.o -c mode7.c





#### IMAGES

#pi_logo.o:	pi_logo.pcx
#	$(CROSS)ld -r -b binary pi_logo.pcx -o pi_logo.o

#pi_logo.x86.o:	pi_logo.pcx
#	ld -r -b binary pi_logo.pcx -o pi_logo.x86.o

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

