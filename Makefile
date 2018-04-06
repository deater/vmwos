include Makefile.inc

all:	kernel/kernel.img

kernel/kernel.img:
	cd elf2bflt && make
	cd userspace && make image
	cd kernel && make

clean:	
	cd kernel && make clean
	cd userspace && make clean
	cd tools && make clean
	cd elf2bflt && make clean
	rm -f *~

