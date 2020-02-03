include ../Makefile.inc

include Makefile_user.inc

GENROMFS = genromfs

all:	$(ELF2BFLT) \
	shell \
	cat cat_count cause_error core_poke cp df hello hexdump ll ls \
	memory_test nano printa printb pwd sysinfo tbo uname \
	chiptune chiptune6
	cd demo && $(MAKE)

image:	shell \
	cat cat_count cause_error core_poke cp df hello hexdump ll ls \
	memory_test nano printa printb pwd sysinfo tbo uname chiptune chiptune6
	mkdir -p image
	mkdir -p image/bin
	mkdir -p image/home
	mkdir -p image/home/ansis
	mkdir -p image/proc
	mkdir -p image/etc
	cp shell cat cat_count cause_error core_poke cp df \
		hello hexdump ll ls memory_test nano printa printb pwd \
		sysinfo tbo uname chiptune chiptune6 ./image/bin
	cp demo/demosplash2019 ./image/bin
	cp CATME ./image/home
	cp files/cpuinfo ./image/proc
	cp files/fstab ./image/etc
	$(GENROMFS) -f initrd.romfs -d image -V VMWos
	echo "unsigned char initrd_image[]={" > initrd.h
	xxd -i < initrd.romfs >> initrd.h
	echo "};" >> initrd.h
	echo "unsigned char initrd2_image[]={" > initrd2.h
	xxd -i < pt3_player.dsk >> initrd2.h
	echo "};" >> initrd2.h

$(ELF2BFLT):
	cd ../elf2bflt && make

cat:	cat.elf
	$(ELF2BFLT) cat.elf cat

cat_count:	cat_count.elf
	$(ELF2BFLT) cat_count.elf cat_count

cause_error:	cause_error.elf
	$(ELF2BFLT) cause_error.elf cause_error

chiptune:	chiptune.elf
	$(ELF2BFLT) chiptune.elf chiptune

chiptune6:	chiptune6.elf
	$(ELF2BFLT) chiptune6.elf chiptune6

core_poke:	core_poke.elf
	$(ELF2BFLT) core_poke.elf core_poke

cp:	cp.elf
	$(ELF2BFLT) cp.elf cp

df:	df.elf
	$(ELF2BFLT) df.elf df

hello:	hello.elf
	$(ELF2BFLT) hello.elf hello

hexdump:	hexdump.elf
	$(ELF2BFLT) hexdump.elf hexdump

ls:	ls.elf
	$(ELF2BFLT) ls.elf ls

ll:	ll.elf
	$(ELF2BFLT) ll.elf ll

memory_test:	memory_test.elf
	$(ELF2BFLT) memory_test.elf memory_test

nano:	nano.elf
	$(ELF2BFLT) nano.elf nano

printa:	printa.elf
	$(ELF2BFLT) printa.elf printa

printb:	printb.elf
	$(ELF2BFLT) printb.elf printb

pwd:	pwd.elf
	$(ELF2BFLT) pwd.elf pwd

shell:	shell.elf
	$(ELF2BFLT) shell.elf shell

slideshow:	slideshow.elf
	$(ELF2BFLT) slideshow.elf slideshow

sysinfo:	sysinfo.elf
	$(ELF2BFLT) sysinfo.elf sysinfo

tbo:	tbo.elf
	$(ELF2BFLT) tbo.elf tbo

uname:	uname.elf
	$(ELF2BFLT) uname.elf uname

###


shell.elf:	shell.o ./vlibc/vlibc.a
	$(CROSS)$(CC) $(CFLAGS) $(LFLAGS) -o shell.elf shell.o ./vlibc/vlibc.a

cat.elf:	cat.o ./vlibc/vlibc.a
	$(CROSS)$(CC) $(CFLAGS) $(LFLAGS) -o cat.elf cat.o ./vlibc/vlibc.a

cat_count.elf:	cat_count.o ./vlibc/vlibc.a
	$(CROSS)$(CC) $(CFLAGS) $(LFLAGS) -o cat_count.elf cat_count.o ./vlibc/vlibc.a

chiptune.elf:	chiptune.o ay8912.o pt3_lib.o ./vlibc/vlibc.a
	$(CROSS)$(CC) $(CFLAGS) $(LFLAGS) -o chiptune.elf chiptune.o ay8912.o pt3_lib.o ./vlibc/vlibc.a

chiptune6.elf:	chiptune6.o ay8912.o pt3_lib.o ./vlibc/vlibc.a
	$(CROSS)$(CC) $(CFLAGS) $(LFLAGS) -o chiptune6.elf chiptune6.o ay8912.o pt3_lib.o ./vlibc/vlibc.a

cause_error.elf:	cause_error.o ./vlibc/vlibc.a
	$(CROSS)$(CC) $(CFLAGS) $(LFLAGS) -o cause_error.elf cause_error.o ./vlibc/vlibc.a

core_poke.elf:	core_poke.o ./vlibc/vlibc.a
	$(CROSS)$(CC) $(CFLAGS) $(LFLAGS) -o core_poke.elf core_poke.o ./vlibc/vlibc.a

cp.elf:	cp.o ./vlibc/vlibc.a
	$(CROSS)$(CC) $(CFLAGS) $(LFLAGS) -o cp.elf cp.o ./vlibc/vlibc.a

df.elf:	df.o ./vlibc/vlibc.a
	$(CROSS)$(CC) $(CFLAGS) $(LFLAGS) -o df.elf df.o ./vlibc/vlibc.a

hello.elf:	hello.o ./vlibc/vlibc.a
	$(CROSS)$(CC) $(CFLAGS) $(LFLAGS) -o hello.elf hello.o ./vlibc/vlibc.a

hexdump.elf:	hexdump.o ./vlibc/vlibc.a
	$(CROSS)$(CC) $(CFLAGS) $(LFLAGS) -o hexdump.elf hexdump.o ./vlibc/vlibc.a

ls.elf:	ls.o ./vlibc/vlibc.a
	$(CROSS)$(CC) $(CFLAGS) $(LFLAGS) -o ls.elf ls.o ./vlibc/vlibc.a

ll.elf:	ll.o
	$(CROSS)$(CC) $(CFLAGS) $(LFLAGS) -o ll.elf ll.o

memory_test.elf:	memory_test.o
	$(CROSS)$(CC) $(CFLAGS) $(LFLAGS) -o memory_test.elf memory_test.o ./vlibc/vlibc.a

nano.elf:	nano.o
	$(CROSS)$(CC) $(CFLAGS) $(LFLAGS) -o nano.elf nano.o ./vlibc/vlibc.a

printa.elf:	printa.o ./vlibc/vlibc.a
	$(CROSS)$(CC) $(CFLAGS) $(LFLAGS) -o printa.elf printa.o ./vlibc/vlibc.a

printb.elf:	printb.o ./vlibc/vlibc.a
	$(CROSS)$(CC) $(CFLAGS) $(LFLAGS) -o printb.elf printb.o ./vlibc/vlibc.a

pwd.elf:	pwd.o ./vlibc/vlibc.a
	$(CROSS)$(CC) $(CFLAGS) $(LFLAGS) -o pwd.elf pwd.o ./vlibc/vlibc.a

slideshow.elf:	slideshow.o ./vlibc/vlibc.a
	$(CROSS)$(CC) $(CFLAGS) $(LFLAGS) -o slideshow.elf slideshow.o ./vlibc/vlibc.a

sysinfo.elf:	sysinfo.o ./vlibc/vlibc.a
	$(CROSS)$(CC) $(CFLAGS) $(LFLAGS) -o sysinfo.elf sysinfo.o ./vlibc/vlibc.a

tbo.elf:	tbo.o ./vlibc/vlibc.a
	$(CROSS)$(CC) $(CFLAGS) $(LFLAGS) -o tbo.elf tbo.o ./vlibc/vlibc.a

uname.elf:	uname.o ./vlibc/vlibc.a
	$(CROSS)$(CC) $(CFLAGS) $(LFLAGS) -o uname.elf uname.o ./vlibc/vlibc.a

####

cat.o:	cat.c
	$(CROSS)$(CC) $(CFLAGS) -c cat.c

cat_count.o:	cat_count.c
	$(CROSS)$(CC) $(CFLAGS) -c cat_count.c

cause_error.o:	cause_error.c
	$(CROSS)$(CC) $(CFLAGS) -c cause_error.c

chiptune.o:	chiptune.c
	$(CROSS)$(CC) $(CFLAGS) -c chiptune.c

chiptune6.o:	chiptune6.c
	$(CROSS)$(CC) $(CFLAGS) -c chiptune6.c

cp.o:	cp.c
	$(CROSS)$(CC) $(CFLAGS) -c cp.c

ay8912.o:	ay8912.c
	$(CROSS)$(CC) $(CFLAGS) -c ay8912.c

pt3_lib.o:	pt3_lib.c
	$(CROSS)$(CC) $(CFLAGS) -DPT3LIB_EMBEDDED -c pt3_lib.c

core_poke.o:	core_poke.c
	$(CROSS)$(CC) $(CFLAGS) -c core_poke.c

df.o:	df.c
	$(CROSS)$(CC) $(CFLAGS) -c df.c

hello.o:	hello.c
	$(CROSS)$(CC) $(CFLAGS) -c hello.c

hexdump.o:	hexdump.c
	$(CROSS)$(CC) $(CFLAGS) -c hexdump.c

memory_test.o:	memory_test.c
	$(CROSS)$(CC) $(CFLAGS) -c memory_test.c

nano.o:	nano.c
	$(CROSS)$(CC) $(CFLAGS) -c nano.c

printa.o:	printa.c
	$(CROSS)$(CC) $(CFLAGS) -c printa.c

printb.o:	printb.c
	$(CROSS)$(CC) $(CFLAGS) -c printb.c

pwd.o:	pwd.c
	$(CROSS)$(CC) $(CFLAGS) -c pwd.c

slideshow.o:	slideshow.c
	$(CROSS)$(CC) $(CFLAGS) -c slideshow.c

tbo.o:	tbo.c
	$(CROSS)$(CC) $(CFLAGS) -Wno-misleading-indentation -c tbo.c

uname.o:	uname.c
	$(CROSS)$(CC) $(CFLAGS) -c uname.c

shell.o:	shell.c
	$(CROSS)$(CC) $(CFLAGS) -c shell.c

sysinfo.o:	sysinfo.c
	$(CROSS)$(CC) $(CFLAGS) -c sysinfo.c

ls.o:	ls.c
	$(CROSS)$(CC) $(CFLAGS) -c ls.c

ll.o:	ll.arm.eabi.s logo.lzss_new
	$(CROSS)$(AS) -o ll.o ll.arm.eabi.s

./vlibc/vlibc.a:
	cd	vlibc && make

clean:
	rm -f *~ *.o *.elf \
		hello cat cat_count cause_error chiptune chiptune6 \
		core_poke cp df hexdump ll ls memory_test nano \
		printa printb pwd shell slideshow sysinfo tbo uname \
		initrd.h initrd.romfs
	cd vlibc && make clean
	cd demo && make clean
