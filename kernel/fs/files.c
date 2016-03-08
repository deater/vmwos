#include <stdint.h>
#include <stddef.h>

#include "errors.h"

#include "drivers/console/console_io.h"
#include "fs/files.h"
#include "fs/romfs/romfs.h"
#include "lib/printk.h"
#include "lib/string.h"

static int debug=1;

#define MAX_FD	32

struct fd_info_t {
	uint32_t valid;
	uint32_t inode;
	uint32_t file_ptr;
} fd_table[MAX_FD];

int32_t fd_free(uint32_t fd) {

	return -EBADF;
}

int32_t fd_allocate(uint32_t inode) {

	int32_t fd;

	if (debug) printk("Attempting to allocate fd for inode %x\n",inode);

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

	if (debug) printk("### Allocated fd %d\n",fd);

	return fd;
}

int32_t close(uint32_t fd) {

	int32_t result;

	result=fd_free(fd);

	return result;

}


int32_t open(const char *pathname, uint32_t flags, uint32_t mode) {

	int32_t result;
	int32_t inode;

	if (debug) {
		printk("### Trying to open %s\n",pathname);
	}

	inode=romfs_get_inode(pathname);
	if (inode<0) {
		return -ENOENT;
	}

	result=fd_allocate(inode);
	if (result<0) {
		return result;
	}

	if (debug) printk("### opened fd %d\n",result);

	return result;

}

int32_t read(uint32_t fd, void *buf, uint32_t count) {

	int32_t result;


	if (fd==0) {
		result=console_read(buf,count);
	}
	else if (fd>=MAX_FD) {
		return -ENFILE;
	}
	else if (fd_table[fd].valid==0) {
		printk("Attempting to read from unsupported fd %d\n",fd);
		result=-EBADF;
	}
	else {
		if (debug) printk("Attempting to read %d bytes from fd %d into %x\n",count,fd,buf);

		result=romfs_read_file(fd_table[fd].inode,
					fd_table[fd].file_ptr,
					buf,count);
		if (result>0) {
			fd_table[fd].file_ptr+=result;
		}
	}
	return result;
}

int32_t write(uint32_t fd, void *buf, uint32_t count) {

	int32_t result;

	if (fd==2) {
		int i;
		char *string = (char *)buf;
		printk("Writing %d bytes, %d\n",count,string[count-1]);
		for(i=0;i<count;i++) {
			printk("%x ",string[i]);
		}
		printk("\n");
	}

	if ((fd==1) || (fd==2)) {
		result = console_write(buf, count);
	}
	else {
		printk("Attempting to write unsupported fd %d\n",fd);
		result=-EBADF;
	}
	return result;
}

int32_t stat(const char *pathname, struct stat *buf) {

	int32_t inode;
	int32_t result;

	inode=romfs_get_inode(pathname);
	if (inode<0) {
		return -ENOENT;
	}

	result=romfs_stat(inode, buf);

	return result;
}

struct superblock_t superblock_table[8];

int32_t mount(const char *source, const char *target,
        const char *filesystemtype, uint32_t mountflags,
        const void *data) {

	int32_t result=0;

	if (!strncmp(filesystemtype,"romfs",5)) {
		result=romfs_mount(&superblock_table[0]);
	}
	else {
		result=-ENODEV;
	}

	return result;
}


void fd_table_init(void) {
	int i;

	for(i=0;i<MAX_FD;i++) {
		fd_table[i].valid=0;
	}

	/* Special case 0/1/2 (stdin/stdout/stderr) */
	/* FIXME: actually hook them up to be proper fds */
	fd_table[0].valid=1;
	fd_table[1].valid=1;
	fd_table[2].valid=1;

	return;
}

