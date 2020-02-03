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

static int debug=0;


static uint32_t ts(int32_t track, int32_t sector) {

	int sectors_per_track=16; /* usually, older disks had 13 */

	return ((track*sectors_per_track)+sector)*DOS33_BLOCK_SIZE;
}

static uint32_t inode_to_block(uint32_t inode_num) {

	uint32_t track, sector;

	/* We store inode as TRACK(8bits)Sector(8bits)Entry(8bits) */

	track=(inode_num>>16)&0xff;
	sector=(inode_num>>8)&0xff;

	return ts(track,sector);
}



static int32_t dos33fs_find_filename(struct inode_type *dir_inode,
					const char *searchname,
					uint32_t *found_inode_num) {


	int32_t name_length;
	uint32_t inode;
	char current_block[DOS33_BLOCK_SIZE];
	uint32_t block_location,i;
	uint32_t cat_offset=0;
	char filename[31];
	int32_t filename_ptr;
	uint32_t next_t,next_s;

	if (debug) {
		printk("dos33fs: looking for %s in %x\n",searchname,dir_inode);
	}

	/* fake a "." entry */
	if (!strncmp(searchname,".",strlen(searchname))) {
		*found_inode_num=(dir_inode->number|0xff);
		if (debug) printk("dos33fs: faking . %x\n",*found_inode_num);
		return 0;
	}

	/* fake a ".." entry */
	if (!strncmp(searchname,"..",strlen(searchname))) {
		*found_inode_num=(((dir_inode->number)&~0xff)|0xfe);
		if (debug) printk("dos33fs: faking .. %x\n",*found_inode_num);
		return 0;
	}

	next_t=(dir_inode->number>>16)&0xff;
	next_s=(dir_inode->number>>8)&0xff;

	while(1) {
		/* start at starting dir */
		block_location=ts(next_t,next_s);
		dir_inode->sb->block->block_ops->read(dir_inode->sb->block,
				block_location,DOS33_VTOC_SIZE,current_block);

		cat_offset=DOS33_CAT_FIRST_ENTRY;

		for(i=0;i<DOS33_CAT_MAX_ENTRIES;i++) {

			inode=(next_t<<16)|(next_s<<8)|i;

			/* if zero then not allocated */
			if (current_block[cat_offset]==0) continue;
			/* if ff then deleted */
			if (current_block[cat_offset]==0xff) continue;

			/* copy in filename */
			memcpy(filename,
				&current_block[cat_offset+
						DOS33_CAT_OFFSET_FILE_NAME],
				DOS33_MAX_FILENAME_SIZE);
			filename[DOS33_MAX_FILENAME_SIZE]='\0';

			/* end of filename is padded with spaces (0xa0) */
			/* so remove them from string */
			filename_ptr=DOS33_MAX_FILENAME_SIZE-1;
			while((filename_ptr>-1) &&
					(filename[filename_ptr]==0xa0)) {
				filename_ptr--;
			}
			filename_ptr++;
			filename[filename_ptr]='\0';

			/* Apple II stores strings with high bit set, */
			/* so clear those */
			/* also calculate string length */
			name_length=0;
			while(filename[name_length]!=0) {
				filename[name_length]=filename[name_length]&0x7f;
				name_length++;
			}

			if (debug) printk("dos33: trying %s\n",filename);
			if (!strncmp(searchname,filename,strlen(searchname))) {
				if (debug) printk("dos33: %s found %x\n",searchname,inode);
				*found_inode_num=inode;
				return 0;
			}

			cat_offset+=DOS33_CAT_ENTRY_SIZE;
		}

		/* see if there are more directory entries */
		next_t=current_block[DOS33_CAT_NEXT_TRACK];
		next_s=current_block[DOS33_CAT_NEXT_SECTOR];

		if ((next_t==0) && (next_s==0)) {
			if (debug) printk("dos33_find: exit at end\n");
			break;
		}

	}

	if (debug) printk("dos33: %s not found\n",searchname);

	return -1;
}


/* Read data from inode->number into inode */
/* Data comes in partially filled, we just fill in rest */
int32_t dos33_read_inode(struct inode_type *inode) {

	int32_t size=0;
	int32_t type=0;

	uint32_t block_location,cat_entry;

	char current_block[DOS33_BLOCK_SIZE];

	if (debug) {
		printk("dos33: Attempting to stat inode %x\n",inode->number);
	}

	/* special case . */
	if ((inode->number&0xff)==0xff) {
		if (debug) {
			printk("dos33: ri: Special case . inode %x\n",inode->number);
		}
		inode->mode=0666;
		inode->mode|=S_IFDIR;
		return 0;
	}

	/* special case .. */
	if ((inode->number&0xff)==0xfe) {
		if (debug) {
			printk("dos33: ri: Special case .. inode %x\n",inode->number);
		}
		inode->mode=0666;
		inode->mode|=S_IFDIR;
		return 0;
	}


	block_location=inode_to_block(inode->number);
	cat_entry=(inode->number&0xff);
	if (cat_entry>=DOS33_CAT_MAX_ENTRIES) {
		printk("Cat entry %d out of bounds\n",cat_entry);
		return -ENOENT;
	}


	inode->sb->block->block_ops->read(inode->sb->block,
				block_location,DOS33_VTOC_SIZE,current_block);

	/* Default mode is global read/write */
		/* -rw-rw-rw- */
	inode->mode=0666;

	type=current_block[DOS33_CAT_OFFSET_FILE_TYPE+
				DOS33_CAT_FIRST_ENTRY+
					(cat_entry*DOS33_CAT_ENTRY_SIZE)];

	/* See if locked (read-only) */
	if (type&DOS33_FILE_TYPE_LOCKED) {
		inode->mode&=~0222;
	}

	/* regular file */
	inode->mode|=S_IFREG;

	/* default size is only measured in multiples of blocksize */
	/* 16-bit little-endian low/high */
	size=current_block[DOS33_CAT_OFFSET_FILE_LENGTH_L+
				DOS33_CAT_FIRST_ENTRY+
				(cat_entry*DOS33_CAT_ENTRY_SIZE)]+
		(current_block[DOS33_CAT_OFFSET_FILE_LENGTH_H+
				DOS33_CAT_FIRST_ENTRY+
				(cat_entry*DOS33_CAT_ENTRY_SIZE)]<<8);

	inode->blocks=size;
	inode->blocksize=DOS33_BLOCK_SIZE;
	size*=DOS33_BLOCK_SIZE;

	/* Do things based on type */
	switch(type&0x7f) {
		case DOS33_FILE_TYPE_T:	/* text */
			break;
		case DOS33_FILE_TYPE_I:	/* integer basic */
			break;
		case DOS33_FILE_TYPE_A:	/* applesoft basic */
			break;
		case DOS33_FILE_TYPE_B:	/* binary */
			/* Mark executable */
			inode->mode|=0111;

			/* FIXME: actual file size can be found by reading */
			/* first few bytes of executable */
			break;
		case DOS33_FILE_TYPE_S:	/* S? */
		case DOS33_FILE_TYPE_R:	/* Relocatable? */
		case DOS33_FILE_TYPE_A2:/* A again */
		case DOS33_FILE_TYPE_B2:/* B again */
		default:
			break;
	}


	inode->size=size;

	/* timestamp */
	/* DOS3.3 was released in August of 1980 */
	inode->atime=334939200;
	inode->mtime=334939200;
	inode->ctime=334939200;

	return 0;
}


static int32_t dos33_lookup_inode_dir(struct inode_type *dir_inode,
		const char *name) {

	uint32_t inode_found;
	int32_t result;

	if (debug) {
		printk("dos33_lookup_inode_dir: "
				"Trying to get inode for file %s in dir %x\n",
				name,dir_inode->number);
	}

	/* First check for special "." or ".." */
	if (!strncmp(name,"..",strlen(name))) {
		return 0;
	}
	if (!strncmp(name,".",strlen(name))) {
		return 0;
	}


	/* Check to make sure our dir_inode is in fact a dir_inode */
	/* FIXME: ? */

	result= dos33fs_find_filename(dir_inode,name,&inode_found);
	if (result==0) {
		if (debug) printk("Found inode %x\n",inode_found);
		dir_inode->number=inode_found;
	}

	return result;
}


/* Inode number is TRACK<<16 | SECTOR<<8 | ENTRY */
/* . and .. are special cases */

int32_t dos33fs_lookup_inode(struct inode_type *inode, const char *name) {

	const char *ptr;
	char dir[MAX_FILENAME_SIZE];
	int32_t result;

	ptr=name;

	/* called with empty string, which would be / */
	if (ptr[0]=='\0') goto lookup_inode_done;

	while(1) {
		if (debug) {
			printk("dos33_lookup_inode: about to split %s\n",ptr);
		}

		ptr=split_filename(ptr,dir,MAX_FILENAME_SIZE);

		if (debug) {
			printk("dos33 path_part %s\n",dir);
		}

		result=dos33_lookup_inode_dir(inode,dir);
		if (result<0) {
			return result;
		}

                if (ptr==NULL) break;

        }
lookup_inode_done:

	result=dos33_read_inode(inode);

	return result;
}

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

	struct vmwos_dirent *dirent_ptr;

	int32_t name_length;
	uint32_t inode=*current_progress;
	int32_t num_entries=0,current_length=0,total_length=0;
	char current_block[DOS33_BLOCK_SIZE];
	uint32_t block_location,i;
	uint32_t cat_offset=0;
	unsigned char filename[31];
	int32_t filename_ptr;
	int32_t next_t,next_s,next_i;

	dirent_ptr=(struct vmwos_dirent *)buf;

	if (debug) {
		printk("dos33fs_getdents: dir_inode %x current_inode %x\n",
			dir_inode,inode);
	}

	/* special case meaning we finished last time */
	if (*current_progress==0xffffffff) {
		return 0;
	}

	if (*current_progress==0) {
		/* fake up a "." and ".." entry */
		name_length=strlen(".");
		dirent_ptr->d_ino=inode | 0xff;
		dirent_ptr->d_off=current_length;
		dirent_ptr->d_reclen=current_length;
		memcpy(dirent_ptr->d_name,".",name_length);
		dirent_ptr->d_name[name_length]=0;
		num_entries++;
		if (debug) {
			printk("dos33_getdents: "
				"added %s namelen %d reclen %d\n",
				dirent_ptr->d_name,name_length,
				dirent_ptr->d_reclen);
		}
		total_length+=current_length;
		dirent_ptr=(struct vmwos_dirent *)(((char *)dirent_ptr)+current_length);

		name_length=strlen("..");
		dirent_ptr->d_ino=inode | 0xfe;
		dirent_ptr->d_off=current_length;
		dirent_ptr->d_reclen=current_length;
		memcpy(dirent_ptr->d_name,"..",name_length);
		dirent_ptr->d_name[name_length]=0;
		num_entries++;
		if (debug) {
			printk("dos33_getdents: "
					"added %s namelen %d reclen %d\n",
					dirent_ptr->d_name,name_length,
					dirent_ptr->d_reclen);
		}
		total_length+=current_length;
		dirent_ptr=(struct vmwos_dirent *)(((char *)dirent_ptr)+current_length);
	}

	if (*current_progress==0) {
		next_t=(dir_inode>>16)&0xff;
		next_s=(dir_inode>>8)&0xff;
		next_i=0;
	}
	else {
		next_t=(*current_progress>>16)&0xff;
		next_s=(*current_progress>>8)&0xff;
		next_i=(*current_progress)&0xff;
	}

	while(1) {
		cat_offset=DOS33_CAT_FIRST_ENTRY;

		/* start from scratch and iterate until we hit current progress */
		block_location=ts(next_t,next_s);
		sb->block->block_ops->read(sb->block,
				block_location,DOS33_VTOC_SIZE,current_block);

		for(i=next_i;i<DOS33_CAT_MAX_ENTRIES;i++) {

			inode=(next_t<<16)|(next_s<<8)|i;

			/* if zero then not allocated */
			/* note: this means track 0 can never be used for data */
			if (current_block[cat_offset]==0) continue;
			/* if ff then deleted */
			if (current_block[cat_offset]==0xff) continue;

			/* copy in filename */
			memcpy(filename,
				&current_block[cat_offset+DOS33_CAT_OFFSET_FILE_NAME],
				DOS33_MAX_FILENAME_SIZE);
			filename[DOS33_MAX_FILENAME_SIZE]='\0';

			/* end of filename is padded with spaces (0xa0) */
			/* so remove them from string */
			filename_ptr=DOS33_MAX_FILENAME_SIZE-1;
			while((filename_ptr>-1) && (filename[filename_ptr]==0xa0)) {
				filename_ptr--;
			}
			filename_ptr++;
			filename[filename_ptr]='\0';

			/* Apple II stores strings with high bit set, so clear those */
			/* also calculate string length */
			name_length=0;
			while(filename[name_length]!=0) {
				filename[name_length]=filename[name_length]&0x7f;
				name_length++;
			}

			/* calculate length of entry */
        	        current_length=(sizeof(uint32_t)*3)+name_length+1;
                	/* pad to integer boundary */
                	if (current_length%4) {
                        	current_length+=4-(current_length%4);
	                }

	                if (current_length+total_length>size) break;

			/* stick things in the struct */
			dirent_ptr->d_ino=inode;
			dirent_ptr->d_off=current_length;
			dirent_ptr->d_reclen=current_length;
			memcpy(dirent_ptr->d_name,filename,name_length);
			dirent_ptr->d_name[name_length]=0;
			num_entries++;
			if (debug) {
				printk("dos33_getdents: "
					"added %s namelen %d reclen %d inode %x\n",
					dirent_ptr->d_name,name_length,
					dirent_ptr->d_reclen,inode);
			}

			cat_offset+=DOS33_CAT_ENTRY_SIZE;
			total_length+=current_length;
			dirent_ptr=(struct vmwos_dirent *)(((char *)dirent_ptr)+current_length);
		}

		/* if too big to fit more, break */
                if (current_length+total_length>size) {
			if (debug) printk("dos33_getdents: exit too big\n");
			break;
		}

		/* see if there are more directory entries */
		next_t=current_block[DOS33_CAT_NEXT_TRACK];
		next_s=current_block[DOS33_CAT_NEXT_SECTOR];
		next_i=0;

		if ((next_t==0) && (next_s==0)) {
			if (debug) printk("dos33_getdents: exit at end\n");
			/* special case meaning we're done */
			inode=0xffffffff;
			break;
		}

		/* follow pointer to next */

	}

	*current_progress=inode;

	if (debug) printk("dos33_getdents: num_entries %d\n",num_entries);

	return total_length;
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
	/* we fake a "." directory entry */
	sb->root_dir=vtoc[DOS33_VTOC_FIRST_CAT_TRACK]<<16 |
			vtoc[DOS33_VTOC_FIRST_CAT_SECTOR]<<8 | 0xff;

	if (debug) {
		printk("Mounted DOS33fs vol %d Tracks %d Sectors %d\n",
			vtoc[DOS33_VTOC_VOLUME],
			vtoc[DOS33_VTOC_NUM_TRACKS],
			vtoc[DOS33_VTOC_NUM_SECTORS]);
	}

	return 0;

}
