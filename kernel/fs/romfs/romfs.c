/* See linux/Documentation/filesystems/romfs.txt */
/* structs are on 16-byte boundaries */

/*            01234567 */
/* Offset: 0: -rom1fs- */
/*         8: size / checksum */
/*        16: volumename (multiple of 16-byte chunks) */
/*        xx: file-headers    */

/* File header info */
/* Offset: 0: next / spec.info    --- next is zero if end		*/
/*			low 4 bits of next has type info and exec bits	*/
/*			all files are uid/guid 0 and all are		*/
/*			globally read/writeable				*/
/*         8: size / checksum */
/*        16: filename (multiple of 16-byte chunks) */
/*        xx: file data */

#include <stddef.h>
#include <stdint.h>

#include "lib/printk.h"
#include "lib/string.h"
#include "lib/endian.h"

#include "drivers/block/ramdisk.h"

#include "fs/files.h"
#include "fs/romfs/romfs.h"

#define MAX_FILENAME_SIZE	256

static int debug=1;

/* offset where files start */
static uint32_t file_headers_start=0;

static int32_t romfs_read_noinc(void *buffer, uint32_t offset, uint32_t size) {

	/* Read from underlying block layer */
	/* FIXME: hardcoded to the ramdisk */
	ramdisk_read(offset,size,buffer);

	return 0;
}

/* romfs strings are nul-terminated and come in 16-byte chunks */
static int32_t romfs_read_string(int32_t offset, char *buffer, int32_t size) {

	char temp_buffer[16];
	int32_t our_offset=offset;
	int32_t max_stringsize=size;
	int32_t max_length;

	/* make the output an empty string */
	buffer[0]=0;

	while(1) {
		/* read 16-byte chunk from the filesystem */
		romfs_read_noinc(temp_buffer,our_offset,16);
		our_offset+=16;

		/* Make sure to not overrun the buffer		*/
		/* note, even if our output string is full	*/
		/* we need to keep iterating so we can		*/
		/* return the total size of the on-disk string	*/
		if (max_stringsize>0) {
			if (max_stringsize>16) {
				max_length=16;
			}
			else {
				max_length=max_stringsize;
			}
			strncpy(buffer,temp_buffer,max_length);
		}

		/* Only exit if hit the end of the string */
		if (temp_buffer[15]==0) break;	/* end of ROMfs string */

		/* adjust the stringsize */
		max_stringsize-=16;
	}
	return our_offset-offset;
}


int32_t romfs_stat(int32_t inode, struct stat *buf) {

	int32_t header_offset,size,temp_int,read_count=0;
	int32_t spec_info=0,type=0;

	if (debug) printk("romfs: Attempting to stat inode %x\n",inode);

	/* Clear all to zero */
	memset(buf,0,sizeof(struct stat));

	/* Set inode value */
	buf->st_ino=inode;

	/* Default mode is global read/write */
		/* -rw-rw-rw- */
	buf->st_mode=0666;


	header_offset=inode;		/* 0: Next */
	romfs_read_noinc(&temp_int,header_offset,4);
	type=ntohl(temp_int);

	/* check if executable */
	if (type&0x8) {
		buf->st_mode|=0111;
	}
	type&=0x7;

	header_offset+=4;		/* 4: spec.info */
	romfs_read_noinc(&temp_int,header_offset,4);
	spec_info=ntohl(temp_int);
	/* FIXME: set proper fields if it's a char or block device? */
	(void)spec_info;


	header_offset+=4;		/* 8: Size */
	romfs_read_noinc(&temp_int,header_offset,4);
	size=ntohl(temp_int);
	buf->st_size=size;

	header_offset+=4;		/* 12: Checksum */


	header_offset+=4;		/* 16: filename */

	return read_count;

}

/* We cheat and just use the file header offset as the inode */
int32_t romfs_get_inode(const char *name) {

	int temp_int;
	int32_t inode=0,next=0;
	uint32_t offset=file_headers_start;
	char filename[MAX_FILENAME_SIZE];

	if (debug) printk("romfs: Trying to get inode for file %s\n",name);

	while(1) {
		inode=offset;

		/* Get pointer to next file */
		romfs_read_noinc(&temp_int,offset,4);
		next=ntohl(temp_int)&~0xf;

		/* Get current filename, which is in chunks of 16 bytes */
		offset+=16;

		romfs_read_string(offset,filename,MAX_FILENAME_SIZE);
		if (debug) printk("%s is %s? %x\n",name,filename,inode);

		/* Match filename */
		if (!strncmp(name,filename,strlen(name))) {
			return inode;
		}

		offset=next;

		if (next==0) break;
	}

	return -1;
}


int32_t romfs_mount(void) {

	int temp_int;
	struct romfs_header_t header;
	uint32_t offset=0;
	int32_t result=0;

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
	/* FIXME: We ignore anything more than 16-bytes */
	/* We really don't care about volume name */
	result=romfs_read_string(offset,header.volume_name,16);
	offset+=result;
	if (debug) {
		printk("\tVolume: %s, file_headers start at %x\n",
			header.volume_name,offset);
	}

	file_headers_start=offset;

	return 0;

}


int32_t romfs_read_file(uint32_t inode,
			uint32_t file_offset,
			void *buf,uint32_t count) {

	int32_t header_offset,size,temp_int,name_length,read_count=0;
	int32_t max_count=0;

	if (debug) printk("romfs: Attempting to read %d bytes from inode %x offset %d\n",
			count,inode,file_offset);


	header_offset=inode;		/* 0: Next */

	header_offset+=4;		/* 4: type */

	header_offset+=4;		/* 8: Size */
	romfs_read_noinc(&temp_int,header_offset,4);
	size=ntohl(temp_int);

	header_offset+=4;		/* 12: Checksum */


	header_offset+=4;		/* 16: filename */
	name_length=romfs_read_string(header_offset,NULL,0);
	header_offset+=name_length;
	if (debug) printk("romfs: inode %d name_length %d header_offset %d\n",
			inode,name_length,header_offset);

	/* Return data */
	/* FIXME: copy_to_user() */

	if (debug) printk("romfs: max count %d, size is %d\n",count,size);
	max_count=count;
	if (max_count>size-file_offset) {
		max_count=size-file_offset;
		if (debug) printk("romfs: count is past end of file, limiting to %d\n",max_count);
	}

	if (debug) printk("romfs: reading %d bytes from %d into %x\n",
		max_count,header_offset+file_offset,buf);

	read_count=ramdisk_read(header_offset+file_offset,max_count,buf);

	if (debug) printk("romfs: result was %d\n",read_count);

	return read_count;
}
