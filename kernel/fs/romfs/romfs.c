/* See linux/Documentation/filesystems/romfs.txt */
/* structs are on 16-byte boundaries */

/*            01234567 */
/* Offset: 0: -rom1fs- */
/*         8: size / checksum */
/*        16: volumename (multiple of 16-byte chunks) */
/*        xx: file-headers    */

#include <stddef.h>
#include <stdint.h>

#include "memory.h"

#include "lib/printk.h"
#include "lib/string.h"
#include "lib/endian.h"

#include "drivers/block/ramdisk.h"

#include "fs/romfs/romfs.h"

#define MAX_FILENAME_SIZE	256

static int debug=1;

/* offset where files start at */
static uint32_t file_headers_start=0;

int romfs_read(void *buffer, uint32_t *offset, uint32_t size) {

	/* Read from underlying block layer */
	/* FIXME: hardcoded to the ramdisk */
	ramdisk_read(*offset,size,buffer);

	/* Update offset pointer */
	(*offset)+=size;

	return 0;
}

static int romfs_read_noinc(void *buffer, uint32_t offset, uint32_t size) {

	/* Read from underlying block layer */
	/* FIXME: hardcoded to the ramdisk */
	ramdisk_read(offset,size,buffer);

	return 0;
}

int open_romfs_file(char *name,
		struct romfs_file_header_t *file) {

	int debug=0;
	int temp_int;
	unsigned char ch;
	struct romfs_header_t header;
	uint32_t offset=0;

	char buffer[16];
	char filename[MAX_FILENAME_SIZE];

	/* Read header */
	romfs_read(header.magic,&offset,8);
	if (memcmp(header.magic,"-rom1fs-",8)) {
		printk("Wrong magic number!\n");
		return -1;
	}

	if (debug) printk("Found romfs filesystem!\n");

	/* Read size */
	romfs_read(&temp_int,&offset,4);
	header.size=ntohl(temp_int);

	if (debug) printk("Size: %d bytes\n",header.size);

	/* Read checksum */
	romfs_read(&temp_int,&offset,4);
	header.checksum=ntohl(temp_int);

	if (debug) printk("Checksum: %x\n",header.size);
	/* FIXME: validate checksum */

	/* Read volume name */
	/* FIXME, various overflow possibilities */
	/* We only record last 16 bytes in header */
	/* We really don't care about volume name */
	while(1) {
		romfs_read(buffer,&offset,16);
		memcpy(header.volume_name,buffer,16);
		if (buffer[15]==0) break;	/* NUL terminated */
	}
	if (debug) printk("Volume: %s\n",header.volume_name);

	while(1) {
		file->addr=offset;

		/* Next */
		romfs_read(&temp_int,&offset,4);
		file->next=ntohl(temp_int)&~0xf;
		file->type=ntohl(temp_int)&0xf;

		/* Special */
		romfs_read(&temp_int,&offset,4);
		file->special=ntohl(temp_int);
		/* Size */
		romfs_read(&temp_int,&offset,4);
		file->size=ntohl(temp_int);
		/* Checksum */
		romfs_read(&temp_int,&offset,4);
		file->checksum=ntohl(temp_int);

		file->filename_start=offset;
		while(1) {
			romfs_read(buffer,&offset,16);
			if (buffer[15]==0) break;	/* NUL terminated */
		}

		file->data_start=offset;

		offset=file->filename_start;

		ramdisk_read_string(file->filename_start,
				MAX_FILENAME_SIZE,
				filename);

		/* Match filename */
		if (!strncmp(name,filename,strlen(name))) return 0;

		if (debug) {
			while(1) {
				romfs_read(&ch,&offset,1);
				//printk("Read %d at %d\n",ch,offset);
				if (ch==0) break;
				printk("%c",ch);
			}

			printk("\n");
			printk("\tAddr: 0x%x\n",file->addr);
			printk("\tNext: 0x%x\n",file->next);
			printk("\tType: 0x%x\n",file->type);
			printk("\tSize: %d\n",file->size);
			printk("\tChecksum: %x\n",file->checksum);
		}

		offset=file->next;

		if (file->next==0) break;
	}

	return -1;
}

/* We cheat and just use the file header offset as the inode */
uint32_t romfs_get_inode(const char *name) {

	int temp_int;
	uint32_t offset=file_headers_start;
	struct romfs_file_header_t file;

	char buffer[16];
	char filename[MAX_FILENAME_SIZE];

	if (debug) printk("romfs: Trying to get inode for file %s\n",name);

	while(1) {
		file.addr=offset;

		/* Get points to next file */
		romfs_read_noinc(&temp_int,offset,4);
		file.next=ntohl(temp_int)&~0xf;

		/* Get current filename, which is in chunks of 16 bytes */
		offset+=16;
		file.filename_start=offset;
		while(1) {
			romfs_read(buffer,&offset,16);
			if (buffer[15]==0) break;	/* NUL terminated */
		}

		offset=file.filename_start;

		ramdisk_read_string(file.filename_start,
				MAX_FILENAME_SIZE,
				filename);

		if (debug) printk("%s is %s? %x\n",name,filename,offset);

		/* Match filename */
		if (!strncmp(name,filename,strlen(name))) {
			return offset;
		}

		offset=file.next;

		if (file.next==0) break;
	}

	return -1;
}


uint32_t romfs_mount(void) {

	int temp_int;
	struct romfs_header_t header;
	uint32_t offset=0;

	char buffer[16];

	/* Read header */
	romfs_read_noinc(header.magic,offset,8);
	if (memcmp(header.magic,"-rom1fs-",8)) {
		printk("Wrong magic number!\n");
		return -1;
	}
	offset+=8;
	if (debug) printk("Found romfs filesystem!\n");

	/* Read size */
	romfs_read_noinc(&temp_int,offset,4);
	header.size=ntohl(temp_int);
	offset+=4;
	if (debug) printk("\tSize: %d bytes\n",header.size);

	/* Read checksum */
	romfs_read_noinc(&temp_int,offset,4);
	header.checksum=ntohl(temp_int);
	offset+=4;
	/* FIXME: validate checksum */
	if (debug) printk("\tChecksum: %x\n",header.size);


	/* Read volume name */
	/* FIXME, various overflow possibilities */
	/* We only record last 16 bytes in header */
	/* We really don't care about volume name */
	while(1) {
		romfs_read_noinc(buffer,offset,16);
		memcpy(header.volume_name,buffer,16);
		offset+=16;
		if (buffer[15]==0) break;	/* NUL terminated */
	}
	if (debug) {
		printk("\tVolume: %s, file_headers start at %x\n",
			header.volume_name,offset);
	}

	file_headers_start=offset;

	return 0;

}


uint32_t romfs_read_file(uint32_t inode,
			uint32_t offset,
			void *buf,uint32_t count) {

	/* FIXME: lots of limit checks */

	memcpy(buf,(void *)inode+offset,count);

	return count;
}
