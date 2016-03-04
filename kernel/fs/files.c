#include <stdint.h>
#include <stddef.h>

#include "errors.h"

#include "drivers/console/console_io.h"
#include "fs/files.h"
#include "fs/romfs/romfs.h"
#include "lib/printk.h"

#define MAX_FD	32

struct fd_info_t {
	uint32_t valid;
	uint32_t inode;
	uint32_t file_ptr;
} fd_table[MAX_FD];

uint32_t fd_free(uint32_t fd) {

	return EBADF;
}

uint32_t fd_allocate(uint32_t inode) {

	uint32_t fd;

	fd=0;
	while(1) {
		if (fd_table[fd].valid==0) {
			fd_table[fd].valid=1;
			fd_table[fd].inode=inode;
			fd_table[fd].file_ptr=0;
			break;
		}
		fd++;
		if (fd>=MAX_FD) {
			fd=-ENFILE;
			break;
		}
	}
	return fd;
}

uint32_t close(uint32_t fd) {

	uint32_t result;

	result=fd_free(fd);

	return result;

}


uint32_t open(const char *pathname, uint32_t flags, uint32_t mode) {

	uint32_t result;
	uint32_t inode;

	inode=romfs_get_inode(pathname);
	if (inode<0) {
		return -ENOENT;
	}

	result=fd_allocate(inode);
	if (result<0) {
		return result;
	}

	return result;

}

uint32_t read(uint32_t fd, void *buf, uint32_t count) {

	uint32_t result;

	if (fd==0) {
		result=console_read(buf,count);
	}
	else if (fd>=MAX_FD) {
		return ENFILE;
	}
	else if (fd_table[fd].valid==0) {
		printk("Attempting to read from unsupported fd %d\n",fd);
		result=-EBADF;
	}
	else {
		result=romfs_read_file(fd_table[fd].inode,
					fd_table[fd].file_ptr,
					buf,count);
	}
	return result;
}

uint32_t write(uint32_t fd, void *buf, uint32_t count) {

	uint32_t result;

	if ((fd==1) || (fd==2)) {
		result = console_write(buf, count);
	}
	else {
		printk("Attempting to write unsupported fd %d\n",fd);
		result=-EBADF;
	}
	return result;
}

void fd_table_int(void) {
	int i;

	for(i=0;i<MAX_FD;i++) {
		fd_table[i].valid=0;
	}

	return;
}
