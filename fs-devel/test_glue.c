#include <stdint.h>
#include <stdarg.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int32_t mount_syscall(const char *source, const char *target,
        const char *filesystemtype, uint32_t mountflags,
        const void *data);

#define NUM_CORES 1

struct process_control_block_type *current_proc[NUM_CORES];

int printk(const char *format, ...) {

	int result;

	va_list argp;
	va_start(argp, format);
	result=vprintf(format, argp);
	va_end(argp);

	return result;
}

static int ramdisk_fd=-1;

int ramdisk_read(int offset,int size,char *buffer) {

	int result;

	if (ramdisk_fd==-1) {
		fprintf(stderr,"Error!\n");
		exit(1);
	}

	lseek(ramdisk_fd,offset,SEEK_SET);
	result=read(ramdisk_fd,buffer,size);

	return result;
}

int get_cpu(void) {
	return 0;
}

int console_write(char *buf, int count) {
	int result;

	result=write(1,buf,count);

	return result;
}

int console_read(char *buf, int count) {
	int result;

	result=read(0,buf,count);

	return result;
}

int test_glue_setup(void) {
	/* Set up the ramdisk */
	ramdisk_fd=open("initrd.romfs",O_RDWR);
	if (ramdisk_fd<0) {
		fprintf(stderr,"Error opening ramdisk\n");
		exit(1);
	}

	/* Mount the ramdisk */
	mount_syscall("/dev/ramdisk","/","romfs",0,NULL);

	return 0;
}
