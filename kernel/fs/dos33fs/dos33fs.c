/* Apple II DOS3.3 filesystem */

/* See: Beneath Apple DOS by Worth and Lechner */

#include <stddef.h>
#include <stdint.h>

#include "lib/printk.h"
#include "lib/string.h"
#include "lib/memset.h"
#include "lib/endian.h"
#include "lib/errors.h"

#include "drivers/block.h"

#include "fs/files.h"
#include "fs/inodes.h"
#include "fs/superblock.h"

#include "fs/dos33fs/dos33fs.h"

static int debug=1;

#if 0

/* Read data from inode->number into inode */
/* Data comes in partially filled, we just fill in rest */
int32_t romfs_read_inode(struct inode_type *inode) {

	int32_t header_offset,size,temp_int;
	int32_t spec_info=0,type=0;

retry_inode:

	if (debug) printk("romfs: Attempting to stat inode %x\n",inode->number);

	/* Default mode is global read/write */
		/* -rw-rw-rw- */
	inode->mode=0666;

	header_offset=inode->number;		/* 0: Next */
	romfs_read_noinc(inode->sb,&temp_int,header_offset,4);
	type=ntohl(temp_int);

	/* check if executable */
	if (type&0x8) {
		inode->mode|=0111;
	}

	type&=0x7;

	header_offset+=4;		/* 4: spec.info */
	romfs_read_noinc(inode->sb,&temp_int,header_offset,4);
	spec_info=ntohl(temp_int);

	switch(type) {
		case 0: /* hard link */
			/* spec_info is inode of destination */
			if (debug) printk("HARD LINK to %x\n",spec_info);
			break;
		case 1: /* directory */
			if (debug) printk("DIRECTORY\n");
			inode->mode|=S_IFDIR;
			/* spec_info is first file in subdir's header */
			break;
		case 2: /* regular file */
			/* sinfo must be zero */
			if (debug) printk("REGULAR FILE\n");
			inode->mode|=S_IFREG;
			break;
		case 3: /* symbolic link */
			/* sinfo must be zero */
			if (debug) printk("SYMBOLIC_LINK\n");
			inode->mode|=S_IFLNK;
			break;
		case 4: /* block device */
			if (debug) printk("BLOCK DEVICE\n");
			inode->mode|=S_IFBLK;
			inode->rdev=spec_info;
			break;
		case 5: /* char device */
			if (debug) printk("CHAR DEVICE\n");
			inode->mode|=S_IFCHR;
			inode->rdev=spec_info;
			break;
		case 6: /* socket */
			if (debug) printk("SOCKET\n");
			inode->mode|=S_IFSOCK;
			break;
		case 7: /* fifo */
			if (debug) printk("FIFO\n");
			inode->mode|=S_IFIFO;
			break;
		default:
			break;
	}

	/* was hard link, let's follow it */
	if (type==0) {
		inode->number=spec_info;
		goto retry_inode;
	}

	header_offset+=4;		/* 8: Size */
	romfs_read_noinc(inode->sb,&temp_int,header_offset,4);
	size=ntohl(temp_int);
	inode->size=size;

	header_offset+=4;		/* 12: Checksum */


	header_offset+=4;		/* 16: filename */


	return 0;
}

/* We cheat and just use the file header offset as the inode */
static int32_t romfs_lookup_inode_dir(struct inode_type *dir_inode,
		const char *name) {

	int temp_int;
	int32_t inode_number=0,next=0,spec=0;
	uint32_t offset;
	char filename[ROMFS_MAX_FILENAME_SIZE];

	offset=dir_inode->number;

	if (debug) {
		printk("romfs_lookup_inode_dir: "
				"Trying to get inode for file %s in dir %x\n",
				name,offset);
	}

	/* Check to make sure our dir_inode is in fact a dir_inode */

	romfs_read_noinc(dir_inode->sb,&temp_int,offset,4);
	next=ntohl(temp_int);
	romfs_read_noinc(dir_inode->sb,&temp_int,offset+4,4);
	spec=ntohl(temp_int);

	if ( (next&0x7)!=1) {
		if (debug) {
			printk("romfs_get_inode: inode %x (%x) is not a dir\n",
				next,offset);
		}
		return -ENOTDIR;
	}

	/* first file in directory is pointed to by spec */
	offset=spec;

	while(1) {
		inode_number=offset;

		/* Get pointer to next file */
		romfs_read_noinc(dir_inode->sb,&temp_int,offset,4);
		next=ntohl(temp_int)&~0xf;

		/* Get current filename, which is in chunks of 16 bytes */
		offset+=16;

		romfs_read_string(dir_inode->sb,offset,
					filename,ROMFS_MAX_FILENAME_SIZE);
		if (debug) printk("romfs_get_inode: %s is %s? %x\n",
				name,filename,inode_number);

		/* Match filename */
		if (!strncmp(name,filename,ROMFS_MAX_FILENAME_SIZE)) {

			/* Follow any hard links */
			inode_number=romfs_inode_follow_links(dir_inode->sb,
								inode_number);

			dir_inode->number=inode_number;

			return 0;
		}

		offset=next;

		if (next==0) break;
	}

	return -1;
}
#endif

/* We cheat and just use the file header offset as the inode */
int32_t dos33fs_lookup_inode(struct inode_type *inode, const char *name) {

#if 0
	const char *ptr;
	char dir[MAX_FILENAME_SIZE];
	int32_t result;

	ptr=name;

	/* called with empty string, which would be / */
	if (ptr[0]=='\0') goto lookup_inode_done;

	while(1) {
		if (debug) {
			printk("romfs_lookup_inode: about to split %s\n",ptr);
		}

		ptr=split_filename(ptr,dir,MAX_FILENAME_SIZE);

		if (debug) {
			printk("romfs path_part %s\n",dir);
		}

		result=romfs_lookup_inode_dir(inode,dir);
		if (result<0) {
			return result;
		}

                if (ptr==NULL) break;

        }
lookup_inode_done:

	result=romfs_read_inode(inode);

	return result;
#endif

	return 0;
}

#if 0

static uint32_t romfs_get_size(struct superblock_type *sb) {

	int temp_int;
	uint32_t offset=0;
	struct romfs_header_t header;

	/* Read header */
	romfs_read_noinc(sb,header.magic,offset,8);
	if (memcmp(header.magic,"-rom1fs-",8)) {
		printk("Wrong magic number!\n");
		return -1;
	}
	offset+=8;
	if (debug) printk("Found romfs filesystem!\n");

	/* Read size */
	romfs_read_noinc(sb,&temp_int,offset,4);
	header.size=ntohl(temp_int);
	offset+=4;
	if (debug) printk("\tSize: %d bytes\n",header.size);

	return header.size;

}
#endif

int32_t dos33fs_statfs(struct superblock_type *superblock,
		struct vmwos_statfs *buf) {


	buf->f_type=DOS33_SUPER_MAGIC;	/* type (dos33fs) */
	buf->f_bsize=DOS33_BLOCK_SIZE;	/* blocksize (256 bytes) */
	buf->f_blocks=superblock->size / DOS33_BLOCK_SIZE;
				/* Total data blocks */
	buf->f_bfree=0;		/* Free blocks in filesystem */
	buf->f_bavail=0;	/* Free blocks available to user */
	buf->f_files=10;	/* Total inodes */
	buf->f_ffree=0;		/* Free inodes */
	buf->f_fsid=0;		/* Filesystem ID */
	buf->f_namelen=DOS33_MAX_FILENAME_SIZE;
				/* Maximum length of filenames */
	buf->f_frsize=0;	/* Fragment size */
	buf->f_flags=0;		/* Mount flags */


//	printk("romfs statfs: returning %d/%d bytes as size\n",
//			buf->f_bsize,buf->f_blocks);

	return 0;
}


int32_t dos33fs_read_file(
			struct superblock_type *sb, uint32_t inode,
			char *buf,uint32_t count,
			uint64_t *file_offset) {

#if 0
	int32_t header_offset,size,temp_int,name_length,read_count=0;
	int32_t max_count=0;

	if (debug) printk("dos33fs: Attempting to read %d bytes "
			"from inode %x offset %d\n",
			count,inode,*file_offset);

	return read_count;
#endif

	return 0;
}



int32_t dos33fs_getdents(struct superblock_type *sb,
			uint32_t dir_inode,
			uint64_t *current_progress,
			void *buf,uint32_t size) {

#if 0
	struct vmwos_dirent *dirent_ptr;

	int32_t header_offset,temp_int,name_length,mode,next_header;
	uint32_t inode=*current_progress;
	int32_t num_entries=0,current_length=0,total_length=0;

	dirent_ptr=(struct vmwos_dirent *)buf;

	if (debug) {
		printk("romfs_getdents: dir_inode %x current_inode %x\n",
			dir_inode,inode);
	}
	if (inode==0xffffffff) return 0;

	/* We are the entry itself? */
	if (inode==0) {
		header_offset=dir_inode;	/* 0: Next */
		romfs_read_noinc(sb,&temp_int,header_offset,4);
		mode=ntohl(temp_int)&0x7;
		/* Check to be sure it's a directory */
		if ( mode!=1) {
			printk("romfs_getdents: offset %x inode %x not a directory!\n",
				header_offset,temp_int);
			return -ENOTDIR;
		}
		/* Get inode of first file */
		header_offset+=4;		/* 4: type */
		romfs_read_noinc(sb,&temp_int,header_offset,4);
		inode=ntohl(temp_int);
	}

	while(1) {

		header_offset=inode;		/* 0: Next */
		romfs_read_noinc(sb,&temp_int,header_offset,4);
		next_header=ntohl(temp_int)&~0xf;
		header_offset+=4;		/* 4: type */
		header_offset+=4;		/* 8: Size */
		header_offset+=4;		/* 12: Checksum */
		header_offset+=4;		/* 16: filename */

		name_length=romfs_string_length(sb,header_offset);

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
		romfs_read_string(sb,
				header_offset,dirent_ptr->d_name,name_length);
		dirent_ptr->d_name[name_length]=0;
		num_entries++;
		if (debug) printk("romfs_getdents: added %s namelen %d reclen %d\n",
			dirent_ptr->d_name,name_length,dirent_ptr->d_reclen);
		inode=next_header;
		total_length+=current_length;
		dirent_ptr=(struct vmwos_dirent *)(((char *)dirent_ptr)+current_length);
	}

	*current_progress=inode;

	if (debug) printk("romfs_getdents: num_entries %d\n",num_entries);

	return total_length;
#endif
	return 0;
}


int32_t dos33fs_write_file(
			struct superblock_type *superblock, uint32_t inode,
                        const char *buf,uint32_t count, uint64_t *offset) {

	/* read only filesystem */

	return -EROFS;
}

static struct file_object_operations dos33fs_file_ops= {
        .read   = dos33fs_read_file,
        .write  = dos33fs_write_file,
        .llseek = llseek_generic,
	.getdents = dos33fs_getdents,
};

int32_t dos33fs_setup_fileops(struct file_object *file) {

	file->file_ops=&dos33fs_file_ops;

	return 0;
}

static struct superblock_operations dos33fs_sb_ops = {
	.statfs = dos33fs_statfs,
	.lookup_inode = dos33fs_lookup_inode,
	.setup_fileops = dos33fs_setup_fileops,
};

static uint32_t ts(int32_t track, int32_t sector) {

	int sectors_per_track=16; /* usually, older disks had 13 */

	return ((track*sectors_per_track)+sector)*DOS33_BLOCK_SIZE;
}

static int ones_lookup[16]={
	/* 0x0 = 0000 */ 0,
	/* 0x1 = 0001 */ 1,
	/* 0x2 = 0010 */ 1,
	/* 0x3 = 0011 */ 2,
	/* 0x4 = 0100 */ 1,
	/* 0x5 = 0101 */ 2,
	/* 0x6 = 0110 */ 2,
	/* 0x7 = 0111 */ 3,
	/* 0x8 = 1000 */ 1,
	/* 0x9 = 1001 */ 2,
	/* 0xA = 1010 */ 2,
	/* 0xB = 1011 */ 3,
	/* 0xC = 1100 */ 2,
	/* 0xd = 1101 */ 3,
	/* 0xe = 1110 */ 3,
	/* 0xf = 1111 */ 4,
};


static void dos33_get_blocks_free(char *vtoc, int32_t *blocks,
						int32_t *blocks_free) {

	unsigned char bitmap[4];
	int i,sectors_free=0;
	int tracks_per_disk,sectors_per_disk;

	tracks_per_disk=vtoc[DOS33_VTOC_NUM_TRACKS];
	sectors_per_disk=vtoc[DOS33_VTOC_NUM_SECTORS];

	for(i=0;i<tracks_per_disk;i++) {
		bitmap[0]=vtoc[DOS33_VTOC_FREE_BITMAPS+(i*4)];
		bitmap[1]=vtoc[DOS33_VTOC_FREE_BITMAPS+(i*4)+1];

                sectors_free+=ones_lookup[bitmap[0]&0xf];
                sectors_free+=ones_lookup[(bitmap[0]>>4)&0xf];
                sectors_free+=ones_lookup[bitmap[1]&0xf];
                sectors_free+=ones_lookup[(bitmap[1]>>4)&0xf];
        }

	*blocks=tracks_per_disk*sectors_per_disk;
        *blocks_free=sectors_free;
}


/* Note, should allocate one of these for each mounted fs */
static char vtoc_list[DOS33_VTOC_SIZE][DOS33_MAX_VTOCS];	/* 256 bytes */
static int32_t next_vtoc=0;

int32_t dos33fs_mount(struct superblock_type *sb,
					struct block_dev_type *block) {


	uint32_t vtoc_location = ts(17,0);	/* Usually at 17:0 */
	char *vtoc;
	int32_t blocks, blocks_free;

	/* Set up block device pointer */
	sb->block=block;

	if (next_vtoc>=DOS33_MAX_VTOCS)	{
		return -E2BIG;
	}

	/* Check magic number? */
	/* Not sure if we have one... */

	vtoc=vtoc_list[next_vtoc];
	sb->private=vtoc;
	next_vtoc++;

	sb->block->block_ops->read(sb->block,
				vtoc_location,DOS33_VTOC_SIZE,vtoc);


	dos33_get_blocks_free(vtoc,&blocks,&blocks_free);

	/* Set size */
	sb->size=blocks*DOS33_BLOCK_SIZE;

	/* Point to our superblock operations */
	sb->sb_ops=dos33fs_sb_ops;

	/* point to root dir of filesystem */
	sb->root_dir=0;

	if (debug) {
		printk("Mounted DOS33fs vol %d Tracks %d Sectors %d\n",
			vtoc[DOS33_VTOC_VOLUME],
			vtoc[DOS33_VTOC_NUM_TRACKS],
			vtoc[DOS33_VTOC_NUM_SECTORS]);
	}

	return 0;

}
