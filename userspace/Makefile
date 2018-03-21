include ../Makefile.inc

include Makefile_user.inc

all:	shell \
	cat cat_count cause_error \
	hello ll ls printa printb pwd slideshow sysinfo tbo uname

image:	shell \
	cat cat_count cause_error \
	hello ll ls printa printb pwd slideshow sysinfo tbo uname
	mkdir -p image
	mkdir -p image/bin
	mkdir -p image/home
	mkdir -p image/home/ansis
	cp shell cat cat_count cause_error \
		hello ll ls printa printb pwd slideshow \
		sysinfo tbo uname ./image/bin
	cp CATME ./image/home
	genromfs -f initrd.romfs -d image -V VMWos
	echo "unsigned char initrd_image[]={" > initrd.h
	xxd -i < initrd.romfs >> initrd.h
	echo "};" >> initrd.h

shell:	shell.elf
	$(CROSS)objcopy $(OBJCOPYFLAGS) shell.elf -O binary shell

cat:	cat.elf
	$(CROSS)objcopy $(OBJCOPYFLAGS) cat.elf -O binary cat

cat_count:	cat_count.elf
	$(CROSS)objcopy $(OBJCOPYFLAGS) cat_count.elf -O binary cat_count

cause_error:	cause_error.elf
	$(CROSS)objcopy $(OBJCOPYFLAGS) cause_error.elf -O binary cause_error

hello:	hello.elf
	$(CROSS)objcopy $(OBJCOPYFLAGS) hello.elf -O binary hello

ls:	ls.elf
	$(CROSS)objcopy $(OBJCOPYFLAGS) ls.elf -O binary ls

ll:	ll.elf
	$(CROSS)objcopy $(OBJCOPYFLAGS) ll.elf -O binary ll

printa:	printa.elf
	$(CROSS)objcopy $(OBJCOPYFLAGS) printa.elf -O binary printa

printb:	printb.elf
	$(CROSS)objcopy $(OBJCOPYFLAGS) printb.elf -O binary printb

pwd:	pwd.elf
	$(CROSS)objcopy $(OBJCOPYFLAGS) pwd.elf -O binary pwd

slideshow:	slideshow.elf
	$(CROSS)objcopy $(OBJCOPYFLAGS) slideshow.elf -O binary slideshow

sysinfo:	sysinfo.elf
	$(CROSS)objcopy $(OBJCOPYFLAGS) sysinfo.elf -O binary sysinfo

tbo:	tbo.elf
	$(CROSS)objcopy $(OBJCOPYFLAGS) tbo.elf -O binary tbo

uname:	uname.elf
	$(CROSS)objcopy $(OBJCOPYFLAGS) uname.elf -O binary uname



shell.elf:	shell.o ./vlibc/vlibc.a
	$(CROSS)$(CC) $(CFLAGS) $(LFLAGS) -o shell.elf shell.o ./vlibc/vlibc.a

cat.elf:	cat.o ./vlibc/vlibc.a
	$(CROSS)$(CC) $(CFLAGS) $(LFLAGS) -o cat.elf cat.o ./vlibc/vlibc.a

cat_count.elf:	cat_count.o ./vlibc/vlibc.a
	$(CROSS)$(CC) $(CFLAGS) $(LFLAGS) -o cat_count.elf cat_count.o ./vlibc/vlibc.a

cause_error.elf:	cause_error.o ./vlibc/vlibc.a
	$(CROSS)$(CC) $(CFLAGS) $(LFLAGS) -o cause_error.elf cause_error.o ./vlibc/vlibc.a

hello.elf:	hello.o ./vlibc/vlibc.a
	$(CROSS)$(CC) $(CFLAGS) $(LFLAGS) -o hello.elf hello.o ./vlibc/vlibc.a

ls.elf:	ls.o ./vlibc/vlibc.a
	$(CROSS)$(CC) $(CFLAGS) $(LFLAGS) -o ls.elf ls.o ./vlibc/vlibc.a

ll.elf:	ll.o
	$(CROSS)$(CC) $(CFLAGS) $(LFLAGS) -o ll.elf ll.o

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

hello.o:	hello.c
	$(CROSS)$(CC) $(CFLAGS) -c hello.c

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
		hello cat cat_count cause_error ll ls printa printb \
		pwd shell slideshow sysinfo tbo uname \
		initrd.h initrd.romfs
	cd vlibc && make clean

