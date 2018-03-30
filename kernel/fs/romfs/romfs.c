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
#include "lib/memset.h"
#include "lib/endian.h"
#include "lib/errors.h"

#include "drivers/block/ramdisk.h"

#include "fs/files.h"
#include "fs/romfs/romfs.h"

#define MAX_FILENAME_SIZE	256

static int debug=0;

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
			strncat(buffer,temp_buffer,max_length);
		}

		/* Only exit if hit the end of the string */
		if (temp_buffer[15]==0) break;	/* end of ROMfs string */

		/* adjust the stringsize */
		max_stringsize-=16;
	}
	return our_offset-offset;
}

/* romfs strings are nul-terminated and come in 16-byte chunks */
static int32_t romfs_string_length(int32_t offset) {

	char temp_buffer[16];
	int32_t our_offset=offset;
	int32_t max_length=0;

	while(1) {
		/* read 16-byte chunk from the filesystem */
		romfs_read_noinc(temp_buffer,our_offset,16);
		our_offset+=16;

		/* Only exit if hit the end of the string */
		if (temp_buffer[15]==0) {
			max_length+=strlen(temp_buffer);
			break;	/* end of ROMfs string */
		}

		/* adjust the stringsize */
		max_length+=16;
	}
	return max_length;
}

/* romfs strings are nul-terminated and come in 16-byte chunks */
static int32_t romfs_string_length_chunks(int32_t offset) {

	char temp_buffer[16];
	int32_t our_offset=offset;
	int32_t max_length=0;

	while(1) {
		/* read 16-byte chunk from the filesystem */
		romfs_read_noinc(temp_buffer,our_offset,16);
		our_offset+=16;

		/* Only exit if hit the end of the string */
		if (temp_buffer[15]==0) {
			max_length+=16;
			break;	/* end of ROMfs string */
		}

		/* adjust the stringsize */
		max_length+=16;
	}
	return max_length;
}

static int32_t romfs_inode_follow_links(int32_t inode) {

	int32_t header_offset,temp_int;
	int32_t type,spec_info;

inode_link_loop:
	header_offset=inode;		/* 0: Next */
	romfs_read_noinc(&temp_int,header_offset,4);
	type=ntohl(temp_int);

	type&=0x7;

	header_offset+=4;		/* 4: spec.info */
	romfs_read_noinc(&temp_int,header_offset,4);
	spec_info=ntohl(temp_int);

	if (type==0) {
		inode=spec_info;
		goto inode_link_loop;
	}

	return inode;

}

int32_t romfs_stat(int32_t inode, struct stat *buf) {

	int32_t header_offset,size,temp_int,read_count=0;
	int32_t spec_info=0,type=0;

retry_inode:

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

	switch(type) {
		case 0: /* hard link */
			/* spec_info is inode of destination */
			if (debug) printk("HARD LINK to %x\n",spec_info);
			break;
		case 1: /* directory */
			if (debug) printk("DIRECTORY\n");
			buf->st_mode|=S_IFDIR;
			/* spec_info is first file in subdir's header */
			break;
		case 2: /* regular file */
			/* sinfo must be zero */
			if (debug) printk("REGULAR FILE\n");
			buf->st_mode|=S_IFREG;
			break;
		case 3: /* symbolic link */
			/* sinfo must be zero */
			if (debug) printk("SYMBOLIC_LINK\n");
			buf->st_mode|=S_IFLNK;
			break;
		case 4: /* block device */
			if (debug) printk("BLOCK DEVICE\n");
			buf->st_mode|=S_IFBLK;
			buf->st_rdev=spec_info;
			break;
		case 5: /* char device */
			if (debug) printk("CHAR DEVICE\n");
			buf->st_mode|=S_IFCHR;
			buf->st_rdev=spec_info;
			break;
		case 6: /* socket */
			if (debug) printk("SOCKET\n");
			buf->st_mode|=S_IFSOCK;
			break;
		case 7: /* fifo */
			if (debug) printk("FIFO\n");
			buf->st_mode|=S_IFIFO;
			break;
		default:
			break;
	}

	/* was hard link, let's follow it */
	if (type==0) {
		inode=spec_info;
		goto retry_inode;
	}

	header_offset+=4;		/* 8: Size */
	romfs_read_noinc(&temp_int,header_offset,4);
	size=ntohl(temp_int);
	buf->st_size=size;

	header_offset+=4;		/* 12: Checksum */


	header_offset+=4;		/* 16: filename */

	return read_count;

}

/* We cheat and just use the file header offset as the inode */
int32_t romfs_get_inode(int32_t dir_inode, const char *name) {

	int temp_int;
	int32_t inode=0,next=0,spec=0;
	uint32_t offset=dir_inode; /* file_headers_start; */
	char filename[MAX_FILENAME_SIZE];

	if (debug) {
		printk("romfs_get_inode: Trying to get inode for file %s\n",name);
	}

	/* Check to make sure our dir_inode is in fact a dir_inode */

	romfs_read_noinc(&temp_int,offset,4);
	next=ntohl(temp_int);
	romfs_read_noinc(&temp_int,offset+4,4);
	spec=ntohl(temp_int);

	if ( (next&0x7)!=1) {
		if (debug) {
			printk("romfs_get_inode: inode %x (%x) is not a dir\n",
				next,dir_inode);
		}
		return -ENOTDIR;
	}

	/* first file in directory is pointed to by spec */
	offset=spec;

	while(1) {
		inode=offset;

		/* Get pointer to next file */
		romfs_read_noinc(&temp_int,offset,4);
		next=ntohl(temp_int)&~0xf;

		/* Get current filename, which is in chunks of 16 bytes */
		offset+=16;

		romfs_read_string(offset,filename,MAX_FILENAME_SIZE);
		if (debug) printk("romfs_get_inode: %s is %s? %x\n",name,filename,inode);

		/* Match filename */
		if (!strncmp(name,filename,MAX_FILENAME_SIZE)) {

			/* Follow any hard links */
			inode=romfs_inode_follow_links(inode);

			return inode;
		}

		offset=next;

		if (next==0) break;
	}

	return -1;
}

static uint32_t romfs_get_size(struct superblock_t *superblock) {

	int temp_int;
	uint32_t offset=0;
	struct romfs_header_t header;

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

	return header.size;

}

int32_t romfs_mount(struct superblock_t *superblock) {

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

	/* This is the inode of the root director */
	file_headers_start=offset;

	return file_headers_start;

}

int32_t romfs_statfs(struct superblock_t *superblock, struct statfs *buf) {

	buf->f_type=0x7275;	/* type (romfs) */
	buf->f_bsize=1024;	/* blocksize */
        buf->f_blocks=romfs_get_size(superblock)/1024;
				/* Total data blocks */
        buf->f_bfree=0;		/* Free blocks in filesystem */
	buf->f_bavail=0;	/* Free blocks available to user */
	buf->f_files=10;	/* Total inodes */
	buf->f_ffree=0;		/* Free inodes */
	buf->f_fsid=0;		/* Filesystem ID */
	buf->f_namelen=MAX_FILENAME_SIZE;
				/* Maximum length of filenames */
	buf->f_frsize=0;	/* Fragment size */
	buf->f_flags=0;		/* Mount flags */


//	printk("romfs statfs: returning %d/%d bytes as size\n",
//			buf->f_bsize,buf->f_blocks);


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
	name_length=romfs_string_length_chunks(header_offset);
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


int32_t romfs_getdents(uint32_t dir_inode,
			uint32_t *current_inode,
			void *buf,uint32_t size) {

	struct vmwos_dirent *dirent_ptr;

	int32_t header_offset,temp_int,name_length,mode,next_header;
	uint32_t inode=*current_inode;
	int32_t num_entries=0,current_length=0,total_length=0;

	dirent_ptr=(struct vmwos_dirent *)buf;

	if (debug) {
		printk("romfs_getdents: dir_inode %x current_inode %x\n",
			dir_inode,inode);
	}
	if (*current_inode==0xffffffff) return 0;

	/* We are the entry itself? */
	if (*current_inode==0) {
		header_offset=dir_inode;	/* 0: Next */
		romfs_read_noinc(&temp_int,header_offset,4);
		mode=ntohl(temp_int)&0x7;
		/* Check to be sure it's a directory */
		if ( mode!=1) {
			printk("romfs_getdents: offset %x inode %x not a directory!\n",
				header_offset,temp_int);
			return -ENOTDIR;
		}
		/* Get inode of first file */
		header_offset+=4;		/* 4: type */
		romfs_read_noinc(&temp_int,header_offset,4);
		inode=ntohl(temp_int);
	}

	while(1) {

		header_offset=inode;		/* 0: Next */
		romfs_read_noinc(&temp_int,header_offset,4);
		next_header=ntohl(temp_int)&~0xf;
		header_offset+=4;		/* 4: type */
		header_offset+=4;		/* 8: Size */
		header_offset+=4;		/* 12: Checksum */
		header_offset+=4;		/* 16: filename */

		name_length=romfs_string_length(header_offset);

		if (debug) {
			printk("romfs_getdents: inode %d next %d\n",
				inode,next_header);
		}

		/* NULL terminated */
		current_length=(sizeof(uint32_t)*3)+name_length+1;
		/* pad to integer boundary */
		if (current_length%4) {
			current_length+=4-(current_length%4);
		}

		if (current_length+total_length>size) break;
		if (inode==0) {
			inode=0xffffffff;
			break;
		}
		dirent_ptr->d_ino=inode;
		dirent_ptr->d_off=current_length;
		dirent_ptr->d_reclen=current_length;
		romfs_read_string(header_offset,dirent_ptr->d_name,name_length);
		dirent_ptr->d_name[name_length]=0;
		num_entries++;
		if (debug) printk("romfs_getdents: added %s namelen %d reclen %d\n",
			dirent_ptr->d_name,name_length,dirent_ptr->d_reclen);
		inode=next_header;
		total_length+=current_length;
		dirent_ptr=(struct vmwos_dirent *)(((char *)dirent_ptr)+current_length);
	}

	*current_inode=inode;

	if (debug) printk("romfs_getdents: num_entries %d\n",num_entries);

	return total_length;
}


