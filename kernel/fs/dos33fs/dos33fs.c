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

static uint32_t get_t(int32_t value) {

	value=value/DOS33_BLOCK_SIZE;

	return value>>4;
}

static uint32_t get_s(int32_t value) {

	value=value/DOS33_BLOCK_SIZE;

	return value&0xf;
}


static uint32_t inode_to_block(uint32_t inode_num) {

	uint32_t track, sector;

	/* We store inode as TRACK(8bits)Sector(8bits)Entry(8bits) */

	track=(inode_num>>16)&0xff;
	sector=(inode_num>>8)&0xff;

	return ts(track,sector);
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

	/* should be replaced by "find leading 1" (ffs) instruction */
	/* if available                                       */
static int find_first_one(unsigned char byte) {

	int i=0;

	/* should be handled before calling */
	if (byte==0) return -1;

	while((byte& (0x1<<i))==0) {
		i++;
	}
	return i;
}


static void dos33_update_blocks_free(struct superblock_type *sb) {

	char *vtoc;
	unsigned char bitmap[4];
	int i,sectors_free=0;
	int tracks_per_disk,sectors_per_disk;

	vtoc=sb->private;

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

	sb->blocks=tracks_per_disk*sectors_per_disk;
        sb->blocks_free=sectors_free;

	if (debug) {
		printk("dos33: ubf: blocks_free=%d/%d\n",
			sb->blocks_free,sb->blocks);
	}
}

static void dos33_mark_blocks_free(struct superblock_type *sb,
				int track, int sector) {

	unsigned char bitmap[4];
	char *vtoc;

	vtoc=sb->private;

	if (debug) {
		printk("dos33: mbf: marking t %d s%d as free\n",track,sector);
	}

	if (sector<8) {
		bitmap[1]=vtoc[DOS33_VTOC_FREE_BITMAPS+(track*4)+1];
		bitmap[1]|=(1<<sector);
		vtoc[DOS33_VTOC_FREE_BITMAPS+(track*4)+1]=bitmap[1];
	}
	else if (sector<16) {
		bitmap[0]=vtoc[DOS33_VTOC_FREE_BITMAPS+(track*4)];
		bitmap[0]|=(1<<(sector-8));
		vtoc[DOS33_VTOC_FREE_BITMAPS+(track*4)]=bitmap[0];
	}
	else {
		printk("dos33: mbf ERROR Sector 0x%x out of range! (t=%x)\n",
			sector,track);
		return;
	}

	dos33_update_blocks_free(sb);
}


static void dos33_mark_blocks_used(struct superblock_type *sb,
						int track, int sector) {

	unsigned char bitmap[4];
	char *vtoc;

	vtoc=sb->private;

	if (sector<8) {
		bitmap[1]=vtoc[DOS33_VTOC_FREE_BITMAPS+(track*4)+1];
		bitmap[1]&=~(1<<sector);
		vtoc[DOS33_VTOC_FREE_BITMAPS+(track*4)+1]=bitmap[1];
	}
	else if (sector<16) {
		bitmap[0]=vtoc[DOS33_VTOC_FREE_BITMAPS+(track*4)];
		bitmap[0]&=~(1<<(sector-8));
		vtoc[DOS33_VTOC_FREE_BITMAPS+(track*4)]=bitmap[0];
	}
	else {
		printk("dos33: mbf ERROR Sector %d out of range!\n",sector);
		return;
	}

	dos33_update_blocks_free(sb);
}



static int32_t dos33_zero_out_sector(struct inode_type *inode,
	uint32_t track, uint32_t sector) {

	char data_sector[DOS33_BLOCK_SIZE];
	int block_location;

	block_location=ts(track,sector);
	memset(data_sector,0,DOS33_BLOCK_SIZE);

	inode->sb->block->block_ops->write(inode->sb->block,
				block_location,DOS33_BLOCK_SIZE,data_sector);

	return 0;
}


static int32_t dos33_allocate_sector(struct inode_type *inode,
					uint32_t *track, uint32_t *sector) {

	int found_track=0,found_sector=0;
	unsigned char bitmap[4];
	int current_track,start_track,track_alloc_direction,byte;
	char *vtoc;
	int tracks_per_disk;

	vtoc=inode->sb->private;


	/* Original algorithm tried to keep allocated tracks together */
	/* near the center of the disk, by tracking last track and */
	/* direction.  This was for performance reasons. */
	/* This is sort of low priority for us */

	start_track=vtoc[DOS33_VTOC_LAST_TRACK_ALLOCATED];
	track_alloc_direction=vtoc[DOS33_VTOC_ALLOC_DIRECTION];
	tracks_per_disk=vtoc[DOS33_VTOC_NUM_TRACKS];

	if (track_alloc_direction==0xff) track_alloc_direction=-1;

	if ((track_alloc_direction!=1) && (track_alloc_direction!=-1)) {
		printk("DOS33: ERROR!  Invalid track dir %i\n",
			track_alloc_direction);
	}


	current_track=start_track;
	found_track=-1;

	while(1) {
		/* Want to check byte 1 first, then byte 0 */
		for(byte=1;byte>-1;byte--) {
			bitmap[byte]=vtoc[DOS33_VTOC_FREE_BITMAPS+
							(current_track*4)+byte];
                        if (bitmap[byte]!=0x00) {
                                found_sector=find_first_one(bitmap[byte]);
                                found_sector+=(8*(1-byte));
                                found_track=current_track;
				break;
                        }
                }

		/* if found, exit */
		if (found_track!=-1) break;

		/* Move to next track, handling overflows */
		current_track+=track_alloc_direction;
		if (current_track<0) {
			current_track=DOS33_VTOC_TRACK;
			track_alloc_direction=1;
		}

		if (current_track>=tracks_per_disk) {
                        current_track=DOS33_VTOC_TRACK;
                        track_alloc_direction=-1;
                }

		if (current_track==start_track) {
			if (debug) {
				printk("DOS33: out of room!\n");
				return -ENOSPC;
			}

		}
        }


	/* clear bit indicating in use */
	dos33_mark_blocks_used(inode->sb,found_track,found_sector);

	/* store new track/direction info */
	vtoc[DOS33_VTOC_LAST_TRACK_ALLOCATED]=found_track;
	vtoc[DOS33_VTOC_ALLOC_DIRECTION]=track_alloc_direction;

	/* Sync vtoc/superblock to disk */
	inode->sb->sb_ops.write_superblock(inode->sb);

	*track=found_track;
	*sector=found_sector;

        return 0;

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
			/* if ff then deleted */
			/* if fe then unlinked */
			if ((current_block[cat_offset]==0) ||
				(current_block[cat_offset]==0xfe) ||
				(current_block[cat_offset]==0xff)) {

				cat_offset+=DOS33_CAT_ENTRY_SIZE;
				continue;
			}

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

static int32_t dos33_get_filesize(struct inode_type *inode,
		char *current_block, int32_t type) {

	int32_t size=0;
	int32_t next_t,next_s,which,location;

	next_t=(inode->number>>16)&0xff;
	next_s=(inode->number>>8)&0xff;
	which=inode->number&0xff;

	/* Load the catalog sector */
	location=ts(next_t,next_s);
	inode->sb->block->block_ops->read(inode->sb->block,
				location,DOS33_BLOCK_SIZE,current_block);

	next_t=current_block[DOS33_CAT_OFFSET_FIRST_T+
			DOS33_CAT_FIRST_ENTRY+(which*DOS33_CAT_ENTRY_SIZE)];
	/* handle unlinked file */
	if (next_t==0xfe) {
		next_t=current_block[DOS33_CAT_OFFSET_END_FILE_NAME+
			DOS33_CAT_FIRST_ENTRY+(which*DOS33_CAT_ENTRY_SIZE)];
	}
	next_s=current_block[DOS33_CAT_OFFSET_FIRST_S+
			DOS33_CAT_FIRST_ENTRY+(which*DOS33_CAT_ENTRY_SIZE)];

	/* Load the first T/S sector */
	location=ts(next_t,next_s);
	inode->sb->block->block_ops->read(inode->sb->block,
				location,DOS33_BLOCK_SIZE,current_block);

	/* These files have the data in the file */
	if ((type==DOS33_FILE_TYPE_B) ||
		(type==DOS33_FILE_TYPE_I) || (type==DOS33_FILE_TYPE_A)) {


		/* load first data block of file */
		next_t=current_block[DOS33_TS_FIRST_TS_T];
		next_s=current_block[DOS33_TS_FIRST_TS_S];

		location=ts(next_t,next_s);
		inode->sb->block->block_ops->read(inode->sb->block,
				location,DOS33_BLOCK_SIZE,current_block);

		/* BASIC programs, size first 2 little endian bytes */
		if ((type==DOS33_FILE_TYPE_I) || (type==DOS33_FILE_TYPE_A)) {
			size=(current_block[0]&0xff)|
				((current_block[1]&0xff)<<8);

			/* file size should include the metadata */
			size+=2;
		}
		/* Binary first 4 bytes are ADDRESS then SIZE (little endian) */
		/* 16-bits. */

		if (type==DOS33_FILE_TYPE_B) {
			size=(current_block[2]&0xff)|
				((current_block[3]&0xff)<<8);
		}
	}
	else {
		/* Type T (and I guess the rest) you only */
		/* get a multiple of sector-size that you find */
		/* by searching the T/S lists */

		/* FIXME: do we care enough to implement this properly? */
		/* I guess if we ever have any LOGO programs? */
	}

	return size;
}


static int32_t dos33_set_filesize(struct inode_type *inode,
		char *current_block, int32_t type, int32_t size) {

	int32_t next_t,next_s,which,location;

	if (debug) {
		printk("dos33: attempting to set filesize to %d\n",size);
	}

	next_t=(inode->number>>16)&0xff;
	next_s=(inode->number>>8)&0xff;
	which=inode->number&0xff;

	/* Load the catalog sector */
	location=ts(next_t,next_s);
	inode->sb->block->block_ops->read(inode->sb->block,
				location,DOS33_BLOCK_SIZE,current_block);

	next_t=current_block[DOS33_CAT_OFFSET_FIRST_T+
			DOS33_CAT_FIRST_ENTRY+(which*DOS33_CAT_ENTRY_SIZE)];
	/* Handle unlinked file */
	if (next_t==0xfe) {
		next_t=current_block[DOS33_CAT_OFFSET_END_FILE_NAME+
			DOS33_CAT_FIRST_ENTRY+(which*DOS33_CAT_ENTRY_SIZE)];
	}

	next_s=current_block[DOS33_CAT_OFFSET_FIRST_S+
			DOS33_CAT_FIRST_ENTRY+(which*DOS33_CAT_ENTRY_SIZE)];

	/* Load the first T/S sector */
	location=ts(next_t,next_s);
	inode->sb->block->block_ops->read(inode->sb->block,
				location,DOS33_BLOCK_SIZE,current_block);

	/* These files have the data in the file */
	if ((type==DOS33_FILE_TYPE_B) ||
		(type==DOS33_FILE_TYPE_I) || (type==DOS33_FILE_TYPE_A)) {


		/* load first data block of file */
		next_t=current_block[DOS33_TS_FIRST_TS_T];
		next_s=current_block[DOS33_TS_FIRST_TS_S];

		location=ts(next_t,next_s);
		inode->sb->block->block_ops->read(inode->sb->block,
				location,DOS33_BLOCK_SIZE,current_block);

		/* BASIC programs, size first 2 little endian bytes */
		if ((type==DOS33_FILE_TYPE_I) || (type==DOS33_FILE_TYPE_A)) {
			current_block[0]=size&0xff;
			current_block[1]=(size>>8)&0xff;
		}
		/* Binary first 4 bytes are ADDRESS then SIZE (little endian) */
		/* 16-bits. */

		if (type==DOS33_FILE_TYPE_B) {
			current_block[2]=size&0xff;
			current_block[3]=(size>>8)&0xff;
		}


		inode->sb->block->block_ops->write(inode->sb->block,
				location,DOS33_BLOCK_SIZE,current_block);

	}
	else {
		/* Type T (and I guess the rest) you only */
		/* get a multiple of sector-size that you find */
		/* by searching the T/S lists */

		/* FIXME: do we care enough to implement this properly? */
		/* I guess if we ever have any LOGO programs? */
	}

	return 0;
}






/* Read data from inode->number into inode */
/* Data comes in partially filled, we just fill in rest */
static int32_t dos33_read_inode(struct inode_type *inode) {

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
		inode->mode=0777;
		inode->mode|=S_IFDIR;
		inode->size=0;
		goto dos33_read_inode_done;
	}

	/* special case .. */
	if ((inode->number&0xff)==0xfe) {
		if (debug) {
			printk("dos33: ri: Special case .. inode %x\n",inode->number);
		}
		inode->mode=0777;
		inode->mode|=S_IFDIR;
		inode->size=0;
		goto dos33_read_inode_done;
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
			break;
		case DOS33_FILE_TYPE_S:	/* S? */
		case DOS33_FILE_TYPE_R:	/* Relocatable? */
		case DOS33_FILE_TYPE_A2:/* A again */
		case DOS33_FILE_TYPE_B2:/* B again */
		default:
			break;
	}



	/* metadata for filesize is stored in the file? */
	/* not really optimal */

	size=dos33_get_filesize(inode,current_block,type&0x7f);

	inode->size=size;

dos33_read_inode_done:
	/* timestamp */
	/* DOS3.3 was released in August of 1980 */
	inode->atime=334939200;
	inode->mtime=334939200;
	inode->ctime=334939200;

	/* zero out other stuff */
	inode->uid=0;
	inode->gid=0;
	inode->rdev=0;
	inode->hard_links=1;

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
	buf->f_blocks=superblock->blocks;
				/* Total data blocks */
	buf->f_bfree=superblock->blocks_free;		/* Free blocks in filesystem */
	buf->f_bavail=0;	/* Free blocks available to user */
	buf->f_files=10;	/* Total inodes */
	buf->f_ffree=0;		/* Free inodes */
	buf->f_fsid=0;		/* Filesystem ID */
	buf->f_namelen=DOS33_MAX_FILENAME_SIZE;
				/* Maximum length of filenames */
	buf->f_frsize=0;	/* Fragment size */
	buf->f_flags=0;		/* Mount flags */

	return 0;
}

int32_t dos33fs_read_file(
			struct inode_type *inode,
			char *buf,uint32_t desired_count,
			uint64_t *file_offset) {

	int32_t read_count=0,type,advance_ts;
	int32_t which_sector,sector_offset;
	uint32_t next_t,next_s,entry,block_location;
	uint32_t data_t,data_s,data_location;
	uint32_t copy_begin,copy_length;
	struct superblock_type *sb;
	uint32_t adjusted_offset;
	int32_t current_sector,last_sector,last_sector_offset;

	char current_block[DOS33_BLOCK_SIZE];
	char current_data[DOS33_BLOCK_SIZE];

	sb=inode->sb;

	if (debug) {
		printk("dos33fs: Attempting to read %d bytes "
			"from inode %x offset %lld\n",
			desired_count,inode->number,*file_offset);
	}

	/* Load the catalog entry */
	next_t=(inode->number>>16)&0xff;
	next_s=(inode->number>>8)&0xff;
	entry=(inode->number&0xff);

	block_location=ts(next_t,next_s);
	sb->block->block_ops->read(sb->block,
				block_location,DOS33_BLOCK_SIZE,current_block);

	next_t=current_block[DOS33_CAT_OFFSET_FIRST_T+
			DOS33_CAT_FIRST_ENTRY+entry*DOS33_CAT_ENTRY_SIZE];
	/* handle unlinked file */
	if (next_t==0xfe) {
		next_t=current_block[DOS33_CAT_OFFSET_END_FILE_NAME+
			DOS33_CAT_FIRST_ENTRY+entry*DOS33_CAT_ENTRY_SIZE];
	}

	next_s=current_block[DOS33_CAT_OFFSET_FIRST_S+
			DOS33_CAT_FIRST_ENTRY+entry*DOS33_CAT_ENTRY_SIZE];
	type=current_block[DOS33_CAT_OFFSET_FILE_TYPE+
			DOS33_CAT_FIRST_ENTRY+entry*DOS33_CAT_ENTRY_SIZE]&0x7f;

	/* For binary files, skip the ADDR/LEN header */
	if (type==DOS33_FILE_TYPE_B) {
		adjusted_offset=*file_offset+4;
		last_sector=(inode->size+4)/DOS33_BLOCK_SIZE;
		last_sector_offset=(inode->size+4)-(last_sector*DOS33_BLOCK_SIZE);
	} else {
		adjusted_offset=*file_offset;
		last_sector=inode->size/DOS33_BLOCK_SIZE;
		last_sector_offset=inode->size-(last_sector*DOS33_BLOCK_SIZE);
	}

	/* If off end of file, return */
	if (*file_offset>inode->size) {
		if (debug) {
			printk("Out of bounds on read, ws=%d fs=%d\n",
				*file_offset,inode->size);
		}
		return 0;
	}



	/* calc offset in file */
	which_sector=(adjusted_offset)/DOS33_BLOCK_SIZE;
	sector_offset=(adjusted_offset)-(which_sector*DOS33_BLOCK_SIZE);

	current_sector=which_sector;

	/* start with new ts list */
	advance_ts=1;

	while(which_sector>=DOS33_MAX_TS_ENTRIES) {
		which_sector-=DOS33_MAX_TS_ENTRIES;
		advance_ts++;
	}

	while(1) {

		while (advance_ts) {
			/* setup new track/sector list page */
			block_location=ts(next_t,next_s);
			if (block_location==0) {
				printk("Error! Off end!\n");
				return -ENOENT;
			}

			/* set up next for next time on the list */
			next_t=current_block[DOS33_TS_NEXT_T];
			next_s=current_block[DOS33_TS_NEXT_S];

			sb->block->block_ops->read(sb->block,
				block_location,DOS33_BLOCK_SIZE,current_block);
			advance_ts--;
		}

		/* open first data list page */
		data_t=current_block[DOS33_TS_FIRST_TS_T+(2*which_sector)];
		data_s=current_block[DOS33_TS_FIRST_TS_S+(2*which_sector)];

		if (debug) {
			printk("Loading data from t:%d s:%d\n",data_t,data_s);
		}

		/* File hole */
		if ((data_t==0) && (data_s==0)) {
			memset(current_data,0,DOS33_BLOCK_SIZE);
		}
		else {
			data_location=ts(data_t,data_s);
			sb->block->block_ops->read(sb->block,
				data_location,DOS33_BLOCK_SIZE,current_data);
		}

		/* make sure copy is in range */

		/* start at sector_offset */
		copy_begin=sector_offset;
		copy_length=DOS33_BLOCK_SIZE-sector_offset;

		/* adjust down to account for requested size */
		if (copy_length>desired_count) {
			copy_length=desired_count;
		}

		/* adjust if we hit end of file */
		if (current_sector==last_sector) {
			if (copy_begin+copy_length>last_sector_offset) {
				copy_length=last_sector_offset-sector_offset;
				desired_count=copy_length;
			}
		}

		memcpy(buf,current_data+copy_begin,copy_length);
		buf+=copy_length;

		/* total bytes read increments by how many bytes copied */
		read_count+=copy_length;

		/* bytes left to copy decremented by bytes we've copied */
		desired_count-=copy_length;

		if (desired_count==0) break;

		/* adjust sector we're reading from */
		current_sector++;
		which_sector++;
		if (which_sector>=DOS33_MAX_TS_ENTRIES) {
			advance_ts++;
			which_sector-=DOS33_MAX_TS_ENTRIES;
		}
		sector_offset=0;
	}

	if (read_count>0) {
		*file_offset+=read_count;
	}

	return read_count;

}

int32_t dos33fs_getdents(struct inode_type *dir_inode,
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

	struct superblock_type *sb;

	sb=dir_inode->sb;


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
		current_length=(sizeof(uint32_t)*3)+name_length+1;
                if (current_length%4) current_length+=4-(current_length%4);
		if (current_length+total_length>size) return -E2BIG;
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
		current_length=(sizeof(uint32_t)*3)+name_length+1;
                if (current_length%4) current_length+=4-(current_length%4);
		if (current_length+total_length>size) return -E2BIG;
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
		next_t=(dir_inode->number>>16)&0xff;
		next_s=(dir_inode->number>>8)&0xff;
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
			/* if ff then deleted */
			/* if fe then unlinked */
			if ((current_block[cat_offset]==0) ||
				(current_block[cat_offset]==0xff) ||
				(current_block[cat_offset]==0xfe)) {
				cat_offset+=DOS33_CAT_ENTRY_SIZE;
				continue;
			}

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


static void dos33fs_write_inode(struct inode_type *inode) {

	int32_t blocksize=0;
	int32_t type=0;

	uint32_t block_location,cat_entry;

	char current_block[DOS33_BLOCK_SIZE];
	char temp_block[DOS33_BLOCK_SIZE];

	if (debug) {
		printk("dos33: Attempting to write inode %x\n",inode->number);
	}

	/* special case ., can't change */
	if ((inode->number&0xff)==0xff) {
		if (debug) {
			printk("dos33: ri: Special case . inode %x\n",inode->number);
		}
		return;
	}

	/* special case .., can't change */
	if ((inode->number&0xff)==0xfe) {
		if (debug) {
			printk("dos33: ri: Special case .. inode %x\n",inode->number);
		}
		return;
	}

	block_location=inode_to_block(inode->number);
	cat_entry=(inode->number&0xff);
	if (cat_entry>=DOS33_CAT_MAX_ENTRIES) {
		printk("Cat entry %d out of bounds\n",cat_entry);
		return;
	}

	/* read current values */
	inode->sb->block->block_ops->read(inode->sb->block,
				block_location,DOS33_VTOC_SIZE,current_block);

	/***************/
	/* Update type */
	/***************/

	type=current_block[DOS33_CAT_OFFSET_FILE_TYPE+
				DOS33_CAT_FIRST_ENTRY+
					(cat_entry*DOS33_CAT_ENTRY_SIZE)];

	/* If writable, then not locked */
	if (inode->mode & 0222) {
		type&=~DOS33_FILE_TYPE_LOCKED;
	} else {
		type|=DOS33_FILE_TYPE_LOCKED;
	}

	current_block[DOS33_CAT_OFFSET_FILE_TYPE+
			DOS33_CAT_FIRST_ENTRY+
				(cat_entry*DOS33_CAT_ENTRY_SIZE)]=type;


	/* update blocksize */
	/* are we setting this the same way DOS33 does */
	/* as it includes all metadata blocks? */

	blocksize=(inode->size%DOS33_BLOCK_SIZE)+1;
	if (blocksize>12) blocksize++;	/* for additional T/S list */
	blocksize++;		/* for cat entry */

	current_block[DOS33_CAT_OFFSET_FILE_LENGTH_L+
				DOS33_CAT_FIRST_ENTRY+
				(cat_entry*DOS33_CAT_ENTRY_SIZE)]=
					blocksize&0xff;

	current_block[DOS33_CAT_OFFSET_FILE_LENGTH_H+
				DOS33_CAT_FIRST_ENTRY+
				(cat_entry*DOS33_CAT_ENTRY_SIZE)]=
					blocksize>>8;

	/* FIXME: some way to change file type to not be B? */

	/* metadata for filesize is stored in the file? */
	/* not really optimal */

	/* use temp_block as we destroy the block we pass in */
	dos33_set_filesize(inode,temp_block,type&0x7f,inode->size);

	/* write it out */
	inode->sb->block->block_ops->write(inode->sb->block,
				block_location,DOS33_VTOC_SIZE,current_block);

	return;
}


/* grow file, with holes */
static int32_t dos33fs_grow_file(struct inode_type *inode, uint64_t new_size) {

	int32_t type,advance_ts;
	uint32_t next_t,next_s,entry,block_location;
	struct superblock_type *sb;
	int32_t last_sector;
	int32_t current_sector,ts_list_offset;
	uint64_t current_size;
	uint32_t track,sector;
	int32_t result;

	char current_block[DOS33_BLOCK_SIZE];

	sb=inode->sb;
	current_size=inode->size;

	if (debug) {
		printk("dos33fs: Attempting to grow %x to size %lld\n",
			inode->number,new_size);
	}

	/* Load the catalog entry */
	next_t=(inode->number>>16)&0xff;
	next_s=(inode->number>>8)&0xff;
	entry=(inode->number&0xff);

	block_location=ts(next_t,next_s);
	sb->block->block_ops->read(sb->block,
				block_location,DOS33_BLOCK_SIZE,current_block);

	next_t=current_block[DOS33_CAT_OFFSET_FIRST_T+
			DOS33_CAT_FIRST_ENTRY+entry*DOS33_CAT_ENTRY_SIZE];
	/* Handle unlinked file */
	if (next_t==0xfe) {
		next_t=current_block[DOS33_CAT_OFFSET_END_FILE_NAME+
			DOS33_CAT_FIRST_ENTRY+entry*DOS33_CAT_ENTRY_SIZE];
	}

	next_s=current_block[DOS33_CAT_OFFSET_FIRST_S+
			DOS33_CAT_FIRST_ENTRY+entry*DOS33_CAT_ENTRY_SIZE];
	type=current_block[DOS33_CAT_OFFSET_FILE_TYPE+
			DOS33_CAT_FIRST_ENTRY+entry*DOS33_CAT_ENTRY_SIZE]&0x7f;

	/* For binary files, skip the ADDR/LEN header */
	if (type==DOS33_FILE_TYPE_B) {
		current_size+=4;
		last_sector=(new_size+4)/DOS33_BLOCK_SIZE;
		//last_sector_offset=(inode->size+4)-(last_sector*DOS33_BLOCK_SIZE);
	} else {
		last_sector=new_size/DOS33_BLOCK_SIZE;
		//last_sector_offset=inode->size-(last_sector*DOS33_BLOCK_SIZE);
	}

	/* calc current offset in file */
	current_sector=(current_size)/DOS33_BLOCK_SIZE;
	ts_list_offset=current_sector;

	/* Check if we haven't grown past file size, if so no need */
	/* to allocate anything new */
	if (current_sector==last_sector) {
		goto grow_file_only_write;
	}


	/* start with new ts list */
	advance_ts=1;

	while(ts_list_offset>=DOS33_MAX_TS_ENTRIES) {
		ts_list_offset-=DOS33_MAX_TS_ENTRIES;
		advance_ts++;
	}

	/* move to right ts list in current file */

	while (advance_ts) {
		/* see if should delete the previous one */

		/* setup new track/sector list page */
		block_location=ts(next_t,next_s);
		if (block_location==0) {
			printk("Error! Off end!\n");
			return -ENOENT;
		}

		/* set up next for next time on the list */
		next_t=current_block[DOS33_TS_NEXT_T];
		next_s=current_block[DOS33_TS_NEXT_S];

		sb->block->block_ops->read(sb->block,
			block_location,DOS33_BLOCK_SIZE,current_block);
		advance_ts--;
	}


	/* now we have ts list for current block */
	/* advance to the ts_list of the new block */
	/* bringing in a new zeroed (full of holes) */
	/* each time we go off the end */

	while(current_sector<last_sector) {

		/* adjust sector we're reading from */
		current_sector++;
		ts_list_offset++;

		/* if we hit the end of the TS list */
		if (ts_list_offset>=DOS33_MAX_TS_ENTRIES) {
			ts_list_offset-=DOS33_MAX_TS_ENTRIES;

			/* allocate new TS entry */
			result=dos33_allocate_sector(inode,&track,&sector);
			if (result<0) {
				goto error_allocating;
			}

			/* zero it out */
			dos33_zero_out_sector(inode,track,sector);

			/* point to next t/s list */
			current_block[DOS33_TS_NEXT_T]=track;
			current_block[DOS33_TS_NEXT_S]=sector;

			/* write out updated t/s list */
			sb->block->block_ops->write(sb->block,
					block_location,DOS33_BLOCK_SIZE,
					current_block);

			/* set up next for next time on the list */
			next_t=track;
			next_s=sector;
			block_location=ts(next_t,next_s);
			sb->block->block_ops->read(sb->block,
				block_location,DOS33_BLOCK_SIZE,current_block);

		}
	}

grow_file_only_write:

	/* update size in inode */
	inode->size=new_size;

	/* Update inode on disk */
	dos33fs_write_inode(inode);

	/* update superblock to disk */
	inode->sb->sb_ops.write_superblock(inode->sb);

	if (debug) {
		printk("dos33fs: set size to %lld\n",new_size);
	}

	return 0;

error_allocating:
	/* FIXME? */
	printk("dos33: error growing and we don't handle it well!\n");

	return result;


}

int32_t dos33fs_write_file(struct inode_type *inode,
                        const char *buf,uint32_t desired_count,
			uint64_t *file_offset) {


	int32_t write_count=0,type,advance_ts;
	int32_t which_sector,sector_offset;
	uint32_t next_t,next_s,entry,block_location;
	uint32_t data_t,data_s,data_location;
	uint32_t copy_begin,copy_length;
	struct superblock_type *sb;
	uint32_t adjusted_offset;
	int32_t current_sector;
	int32_t result;

	char current_block[DOS33_BLOCK_SIZE];
	char current_data[DOS33_BLOCK_SIZE];

	sb=inode->sb;

	if (debug) {
		printk("dos33fs: Attempting to write %d bytes "
			"to inode %x offset %lld\n",
			desired_count,inode->number,*file_offset);
	}

	/* First grow file if not big enough */
	if (*file_offset+desired_count > inode->size) {
		if (debug) {
			printk("dos33fs: write: growing file to %lld bytes\n",
				desired_count+*file_offset);
		}
		result=dos33fs_grow_file(inode,*file_offset+desired_count);
		if (result<0) {
			return result;
		}
	}

	/* Load the catalog entry */
	next_t=(inode->number>>16)&0xff;
	next_s=(inode->number>>8)&0xff;
	entry=(inode->number&0xff);

	block_location=ts(next_t,next_s);
	sb->block->block_ops->read(sb->block,
				block_location,DOS33_BLOCK_SIZE,current_block);

	next_t=current_block[DOS33_CAT_OFFSET_FIRST_T+
			DOS33_CAT_FIRST_ENTRY+entry*DOS33_CAT_ENTRY_SIZE];
	/* Handle unlinked file */
	if (next_t==0xfe) {
		next_t=current_block[DOS33_CAT_OFFSET_END_FILE_NAME+
			DOS33_CAT_FIRST_ENTRY+entry*DOS33_CAT_ENTRY_SIZE];
	}

	next_s=current_block[DOS33_CAT_OFFSET_FIRST_S+
			DOS33_CAT_FIRST_ENTRY+entry*DOS33_CAT_ENTRY_SIZE];
	type=current_block[DOS33_CAT_OFFSET_FILE_TYPE+
			DOS33_CAT_FIRST_ENTRY+entry*DOS33_CAT_ENTRY_SIZE]&0x7f;

	/* For binary files, skip the ADDR/LEN header */
	if (type==DOS33_FILE_TYPE_B) {
		adjusted_offset=*file_offset+4;
	} else {
		adjusted_offset=*file_offset;
	}

	/* calc offset in file */
	which_sector=(adjusted_offset)/DOS33_BLOCK_SIZE;
	sector_offset=(adjusted_offset)-(which_sector*DOS33_BLOCK_SIZE);

	current_sector=which_sector;

	/* start with new ts list */
	advance_ts=1;

	while(which_sector>=DOS33_MAX_TS_ENTRIES) {
		which_sector-=DOS33_MAX_TS_ENTRIES;
		advance_ts++;
	}

	while(1) {

		while (advance_ts) {
			/* setup new track/sector list page */
			block_location=ts(next_t,next_s);
			if (block_location==0) {
				printk("Error! Off end!\n");
				return -ENOENT;
			}

			/* set up next for next time on the list */
			next_t=current_block[DOS33_TS_NEXT_T];
			next_s=current_block[DOS33_TS_NEXT_S];

			sb->block->block_ops->read(sb->block,
				block_location,DOS33_BLOCK_SIZE,current_block);
			advance_ts--;
		}

		/* open first data list page */
		data_t=current_block[DOS33_TS_FIRST_TS_T+(2*which_sector)];
		data_s=current_block[DOS33_TS_FIRST_TS_S+(2*which_sector)];

		if (debug) {
			printk("dos33fs: write: "
				"Loading existing data from t:%d s:%d\n",
				data_t,data_s);
		}

		/* File hole */
		if ((data_t==0) && (data_s==0)) {
			/* need to allocate a block for it */
			result=dos33_allocate_sector(inode,&data_t,&data_s);
			if (result<0) {
				return result;
			}
			/* update the T/S list */
			current_block[DOS33_TS_FIRST_TS_T+(2*which_sector)]=data_t;
			current_block[DOS33_TS_FIRST_TS_S+(2*which_sector)]=data_s;
			sb->block->block_ops->write(sb->block,
				block_location,DOS33_BLOCK_SIZE,current_block);

			data_location=ts(data_t,data_s);
			memset(current_data,0,DOS33_BLOCK_SIZE);
		}
		else {
			data_location=ts(data_t,data_s);
			sb->block->block_ops->read(sb->block,
				data_location,DOS33_BLOCK_SIZE,current_data);
		}

		/* make sure copy is in range */

		/* start at sector_offset */
		copy_begin=sector_offset;
		copy_length=DOS33_BLOCK_SIZE-sector_offset;

		/* adjust down to account for requested size */
		if (copy_length>desired_count) {
			copy_length=desired_count;
		}

		memcpy(current_data+copy_begin,buf,copy_length);

		/* write back out to disk */
		data_location=ts(data_t,data_s);
		sb->block->block_ops->write(sb->block,
				data_location,DOS33_BLOCK_SIZE,current_data);

		buf+=copy_length;

		/* total bytes written increments by how many bytes copied */
		write_count+=copy_length;

		/* bytes left to write decremented by bytes we've copied */
		desired_count-=copy_length;

		if (desired_count==0) break;

		/* adjust sector we're writing to */
		current_sector++;
		which_sector++;
		if (which_sector>=DOS33_MAX_TS_ENTRIES) {
			advance_ts++;
			which_sector-=DOS33_MAX_TS_ENTRIES;
		}
		sector_offset=0;
	}

	if (write_count>0) {
		*file_offset+=write_count;
	}

	return write_count;
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


	/* Find current desired end sector */
	/* write zero to it past offset */
	/* go to end of file, zeroing sectors and freeing */
	/* be sure to update free map in superblock */

static int32_t dos33fs_shrink_file(struct inode_type *inode, uint64_t size) {

	int32_t type,advance_ts;
	int32_t which_sector,sector_offset;
	uint32_t next_t,next_s,entry,block_location;
	uint32_t data_t,data_s,data_location;
	uint32_t zero_begin,zero_length;
	struct superblock_type *sb;
	uint32_t adjusted_offset;
	int32_t last_sector;
	int32_t current_sector;
	int32_t delete_this_ts_list=0;

	char current_block[DOS33_BLOCK_SIZE];
	char current_data[DOS33_BLOCK_SIZE];

	sb=inode->sb;

	if (debug) {
		printk("dos33fs: Attempting to shrink %x to size %lld\n",
			inode->number,size);
	}

	/* Load the catalog entry */
	next_t=(inode->number>>16)&0xff;
	next_s=(inode->number>>8)&0xff;
	entry=(inode->number&0xff);

	block_location=ts(next_t,next_s);
	sb->block->block_ops->read(sb->block,
				block_location,DOS33_BLOCK_SIZE,current_block);

	next_t=current_block[DOS33_CAT_OFFSET_FIRST_T+
			DOS33_CAT_FIRST_ENTRY+entry*DOS33_CAT_ENTRY_SIZE];
	/* Handle unlinked file */
	if (next_t==0xfe) {
		next_t=current_block[DOS33_CAT_OFFSET_END_FILE_NAME+
			DOS33_CAT_FIRST_ENTRY+entry*DOS33_CAT_ENTRY_SIZE];
	}

	next_s=current_block[DOS33_CAT_OFFSET_FIRST_S+
			DOS33_CAT_FIRST_ENTRY+entry*DOS33_CAT_ENTRY_SIZE];
	type=current_block[DOS33_CAT_OFFSET_FILE_TYPE+
			DOS33_CAT_FIRST_ENTRY+entry*DOS33_CAT_ENTRY_SIZE]&0x7f;

	/* For binary files, skip the ADDR/LEN header */
	if (type==DOS33_FILE_TYPE_B) {
		adjusted_offset=size+4;
		last_sector=(inode->size+4)/DOS33_BLOCK_SIZE;
		//last_sector_offset=(inode->size+4)-(last_sector*DOS33_BLOCK_SIZE);
	} else {
		adjusted_offset=size;
		last_sector=inode->size/DOS33_BLOCK_SIZE;
		//last_sector_offset=inode->size-(last_sector*DOS33_BLOCK_SIZE);
	}

	/* This should never happen */
	if (size>inode->size) {
		printk("ERROR: shrink size larger than file\n");
		return -E2BIG;
	}

	/* calc offset in file */
	which_sector=(adjusted_offset)/DOS33_BLOCK_SIZE;
	sector_offset=(adjusted_offset)-(which_sector*DOS33_BLOCK_SIZE);

	current_sector=which_sector;

	/* start with new ts list */
	advance_ts=1;

	while(which_sector>=DOS33_MAX_TS_ENTRIES) {
		which_sector-=DOS33_MAX_TS_ENTRIES;
		advance_ts++;
	}

	while(1) {

		while (advance_ts) {
			/* see if should delete the previous one */
			if (delete_this_ts_list) {
				dos33_mark_blocks_free(sb,
					get_t(block_location),
					get_s(block_location));
			}

			/* setup new track/sector list page */
			block_location=ts(next_t,next_s);
			if (block_location==0) {
				printk("Error! Off end!\n");
				return -ENOENT;
			}

			/* set up next for next time on the list */
			next_t=current_block[DOS33_TS_NEXT_T];
			next_s=current_block[DOS33_TS_NEXT_S];

			sb->block->block_ops->read(sb->block,
				block_location,DOS33_BLOCK_SIZE,current_block);
			advance_ts--;
		}

		/* open data block */
		data_t=current_block[DOS33_TS_FIRST_TS_T+(2*which_sector)];
		data_s=current_block[DOS33_TS_FIRST_TS_S+(2*which_sector)];

		/* File hole */
		if ((data_t==0) && (data_s==0)) {
			/* do nothing */
		}
		else {
			/* zero out data */
			data_location=ts(data_t,data_s);
			sb->block->block_ops->read(sb->block,
					data_location,DOS33_BLOCK_SIZE,
					current_data);

			/* start at sector_offset */
			zero_begin=sector_offset;
			zero_length=DOS33_BLOCK_SIZE-sector_offset;

			if (debug) {
				printk("Zeroing %d bytes at %d at t/s %x\n",
					zero_length,zero_begin,data_location);
			}
			memset(current_data+zero_begin,0,zero_length);

			/* write back out to disk */
			data_location=ts(data_t,data_s);
			sb->block->block_ops->write(sb->block,
					data_location,DOS33_BLOCK_SIZE,
					current_data);

			/* only erase sector if we're not using it */
			if (sector_offset==0) {
				if (debug) {
					printk("Freeing t/s %x entry %d\n",
						block_location,which_sector);
				}
				/* delete sector from t/s list */
				current_block[DOS33_TS_FIRST_TS_T+
					(2*which_sector)]=0;
				current_block[DOS33_TS_FIRST_TS_S+
					(2*which_sector)]=0;

				sb->block->block_ops->write(sb->block,
					block_location,DOS33_BLOCK_SIZE,
					current_block);

				/* update superblock free list */
				dos33_mark_blocks_free(sb,
					get_t(data_location),
					get_s(data_location));
			}
		}

		if (current_sector==last_sector) {

			if (delete_this_ts_list) {
				dos33_mark_blocks_free(sb,
					get_t(block_location),
					get_s(block_location));
			}
			break;
		}

		/* adjust sector we're reading from */
		current_sector++;
		which_sector++;

		/* if we hit the end of the TS list */
		if (which_sector>=DOS33_MAX_TS_ENTRIES) {
			advance_ts++;
			which_sector-=DOS33_MAX_TS_ENTRIES;
			/* we'll want to delete all future ones */
			delete_this_ts_list=1;
			/* remove pointer to next t/s list in current */
			current_block[DOS33_TS_NEXT_T]=0;
			current_block[DOS33_TS_NEXT_S]=0;
			sb->block->block_ops->write(sb->block,
					block_location,DOS33_BLOCK_SIZE,
					current_block);
		}
		sector_offset=0;
	}

	/* update size in inode */
	inode->size=size;

	/* Update inode on disk */
	dos33fs_write_inode(inode);

	/* update superblock to disk */
	inode->sb->sb_ops.write_superblock(inode->sb);

	if (debug) {
		printk("dos33fs: set size to %lld\n",size);
	}

	return 0;

}

static int32_t dos33fs_truncate_inode(struct inode_type *inode, uint64_t size) {

	int32_t result;

	/* If same size, do nothing */
	if (size==inode->size) return 0;

	if (size < inode->size) {
		/* shrinking */
		result=dos33fs_shrink_file(inode,size);
	}
	else {
		result=dos33fs_grow_file(inode,size);
	}

	return result;
}

static int32_t dos33fs_unlink_inode(struct inode_type *inode) {

	int32_t result=0;

	uint32_t block_location,cat_entry,old_track;

	char current_block[DOS33_BLOCK_SIZE];

	if (debug) {
		printk("dos33: Attempting to unlink inode %x\n",inode->number);
	}

	/* special case . , can't remove */
	if ((inode->number&0xff)==0xff) {
		return -EISDIR;
	}

	/* special case .. */
	if ((inode->number&0xff)==0xfe) {
		return -EISDIR;
	}

	block_location=inode_to_block(inode->number);
	cat_entry=(inode->number&0xff);
	if (cat_entry>=DOS33_CAT_MAX_ENTRIES) {
		printk("Cat entry %d out of bounds\n",cat_entry);
		return -ENOENT;
	}

	inode->sb->block->block_ops->read(inode->sb->block,
				block_location,DOS33_BLOCK_SIZE,current_block);

	/* Remove from directory */

	/* Note this is a VMWos hack because the inode and filename */
	/* are one and the same, apple dos might not know what to do */
	/* with such a file */

	old_track=current_block[
			DOS33_CAT_FIRST_ENTRY+(cat_entry*DOS33_CAT_ENTRY_SIZE)+
			DOS33_CAT_OFFSET_FIRST_T];

	current_block[
			DOS33_CAT_FIRST_ENTRY+(cat_entry*DOS33_CAT_ENTRY_SIZE)+
			DOS33_CAT_OFFSET_FIRST_T]=0xfe;

	current_block[
			DOS33_CAT_FIRST_ENTRY+(cat_entry*DOS33_CAT_ENTRY_SIZE)+
			DOS33_CAT_OFFSET_END_FILE_NAME]=old_track;

	inode->sb->block->block_ops->write(inode->sb->block,
				block_location,DOS33_BLOCK_SIZE,current_block);

	return result;
}

#if 0
static void dos33fs_dump_sector(const char *sector) {
	int i;

	for(i=0;i<DOS33_BLOCK_SIZE;i++) {
		if (i%16==0) printk("\n%02x: ",i);
		printk("%02x ",sector[i]);
	}
	printk("\n");
}
#endif


	/* First truncate to 0 */
	/* Then remove T/S list */
	/* Then mark diretctory first track as $FF (deleted) */

	/* Note, DOS33 typically leaves contents of deleted files around */
	/* and marks the catalog entry as DELETED so undelete is possible */
	/* we aren't doing that here */

static int32_t dos33fs_destroy_inode(struct inode_type *inode) {

	int result;

	uint32_t block_location,cat_entry,old_track;
	uint32_t ts_track,ts_sector;//,data_track,data_sector;

	char current_block[DOS33_BLOCK_SIZE];

	if (debug) {
		printk("dos33: Attempting to destroy inode %x\n",
			inode->number);
	}

	/* Reduce to a zero-length file */
	result=dos33fs_truncate_inode(inode, 0);
	if (result<0) {
		printk("dos33: ERROR destroying inode\n");
		return result;
	}

	/* special case . , can't destroy */
	if ((inode->number&0xff)==0xff) {
		return -EISDIR;
	}

	/* special case .. */
	if ((inode->number&0xff)==0xfe) {
		return -EISDIR;
	}

	block_location=inode_to_block(inode->number);
	cat_entry=(inode->number&0xff);
	if (cat_entry>=DOS33_CAT_MAX_ENTRIES) {
		printk("Cat entry %d out of bounds\n",cat_entry);
		return -ENOENT;
	}

	inode->sb->block->block_ops->read(inode->sb->block,
				block_location,DOS33_BLOCK_SIZE,current_block);

	/* Remove from directory */

	old_track=current_block[
			DOS33_CAT_FIRST_ENTRY+(cat_entry*DOS33_CAT_ENTRY_SIZE)+
			DOS33_CAT_OFFSET_FIRST_T];

	if (old_track==0xfe) {
		/* we've already unlinked this file so old_track is */
		/* already in the proper place */
		old_track=current_block[
			DOS33_CAT_FIRST_ENTRY+(cat_entry*DOS33_CAT_ENTRY_SIZE)+
			DOS33_CAT_OFFSET_END_FILE_NAME];

	}
	else {
		current_block[
			DOS33_CAT_FIRST_ENTRY+(cat_entry*DOS33_CAT_ENTRY_SIZE)+
			DOS33_CAT_OFFSET_END_FILE_NAME]=old_track;
	}

	/* mark as deleted */
	current_block[
			DOS33_CAT_FIRST_ENTRY+(cat_entry*DOS33_CAT_ENTRY_SIZE)+
			DOS33_CAT_OFFSET_FIRST_T]=0xff;

	inode->sb->block->block_ops->write(inode->sb->block,
				block_location,DOS33_BLOCK_SIZE,current_block);


	/* get remaining track/sector list */
	ts_track=old_track;
	ts_sector=current_block[
			DOS33_CAT_FIRST_ENTRY+(cat_entry*DOS33_CAT_ENTRY_SIZE)+
			DOS33_CAT_OFFSET_FIRST_S];


	/* delete remaining block */
	/* Note: if we truncate to 0 we don't have to do this? */

#if 0
	inode->sb->block->block_ops->read(inode->sb->block,
				ts(ts_track,ts_sector),DOS33_BLOCK_SIZE,
				current_block);

	data_track=current_block[DOS33_TS_FIRST_TS_T];
	data_sector=current_block[DOS33_TS_FIRST_TS_S];
	dos33_mark_blocks_free(inode->sb,data_track,data_sector);
#endif

	/* delete t/s list and update superblock free list */
	if (debug) {
		printk("deleting last t/s list %x/%x\n",ts_track,ts_sector);
	}
	dos33_mark_blocks_free(inode->sb,ts_track,ts_sector);

	return 0;
}


	/* Create directory entry */
static int32_t dos33fs_make_inode(struct inode_type *dir_inode,
				struct inode_type **new_inode) {

	char current_block[DOS33_BLOCK_SIZE];
	uint32_t block_location,cat_entry;
	uint32_t cat_offset=0;
	uint32_t next_t,next_s,track,sector;
	int32_t found=0,inode_number=0,result;
	int32_t type;

	/* By default make binary file */
	/* FIXME: when/where are permissions set? */
	type=DOS33_FILE_TYPE_B;

	if (debug) {
		printk("dos33fs: making new inode in dir %x\n",
			dir_inode->number);
	}

	next_t=(dir_inode->number>>16)&0xff;
	next_s=(dir_inode->number>>8)&0xff;

	while(1) {
		/* start at starting dir */
		block_location=ts(next_t,next_s);
		dir_inode->sb->block->block_ops->read(dir_inode->sb->block,
				block_location,DOS33_VTOC_SIZE,current_block);

		cat_offset=DOS33_CAT_FIRST_ENTRY;

		for(cat_entry=0;cat_entry<DOS33_CAT_MAX_ENTRIES;cat_entry++) {

			inode_number=(next_t<<16)|(next_s<<8)|cat_entry;

			/* if zero then not allocated */
			/* if ff then deleted */
			/* if fe then unlinked */
			/* To be nice we'd exhaust 0 entries before */
			/* moving on to ff, but it's easier not to */

			if ((current_block[cat_offset]==0) ||
				(current_block[cat_offset]==0xff)) {

				found=1;
				break;
			}

			cat_offset+=DOS33_CAT_ENTRY_SIZE;
		}

		if (found) break;

		/* see if there are more directory entries */
		next_t=current_block[DOS33_CAT_NEXT_TRACK];
		next_s=current_block[DOS33_CAT_NEXT_SECTOR];

		if ((next_t==0) && (next_s==0)) {
			printk("dos33_make_inode: no more room\n");
			/* FIXME: allocate another sector */
			return -E2BIG;
		}

	}

	if (debug) {
		printk("dos33: make_inode found %x\n",inode_number);
	}

	(*new_inode)->number=inode_number;
	(*new_inode)->sb=dir_inode->sb;

	/* create initial T/S list */
	result=dos33_allocate_sector(*new_inode,&track,&sector);
	if (result<0) {
		printk("dos33: error allocating sector for t/s list\n");
		/* FIXME: should free the dir entry we made */
		return result;
	}
	/* zero out the new ts list */
	dos33_zero_out_sector(*new_inode,track,sector);

	if (debug) {
		printk("dos33: make_inode: made new T/S list at t: %x s: %x\n",
			track,sector);
	}

	/* set t/s list */
	current_block[DOS33_CAT_OFFSET_FIRST_T+
				DOS33_CAT_FIRST_ENTRY+
				(cat_entry*DOS33_CAT_ENTRY_SIZE)]=track;
	current_block[DOS33_CAT_OFFSET_FIRST_S+
				DOS33_CAT_FIRST_ENTRY+
				(cat_entry*DOS33_CAT_ENTRY_SIZE)]=sector;
	current_block[DOS33_CAT_OFFSET_FILE_TYPE+
				DOS33_CAT_FIRST_ENTRY+
				(cat_entry*DOS33_CAT_ENTRY_SIZE)]=type;

	/* write out updated catalog entry */
	dir_inode->sb->block->block_ops->write(dir_inode->sb->block,
				block_location,DOS33_BLOCK_SIZE,current_block);

	/* set block_location to new t/s list */
	block_location=ts(track,sector);


	/* create initial data block */
	/* strictly shouldn't be necessary, but we store file size */
	/* at start of data block */

	result=dos33_allocate_sector(*new_inode,&track,&sector);
	if (result<0) {
		printk("dos33: error allocating sector for data\n");
		/* FIXME: should free the dir entry we made */
		return result;
	}
	/* zero out the new data block  */
	dos33_zero_out_sector(*new_inode,track,sector);

	if (debug) {
		printk("dos33: make_inode: made initial data block at "
			"t: %x s: %x\n",track,sector);
	}

	/* put new data block in new  t/s list */
	memset(current_block,0,DOS33_BLOCK_SIZE);
	current_block[DOS33_TS_FIRST_TS_T]=track;
	current_block[DOS33_TS_FIRST_TS_S]=sector;

	/* write out new t/s list */
	dir_inode->sb->block->block_ops->write(dir_inode->sb->block,
				block_location,DOS33_BLOCK_SIZE,current_block);

	/* set filesize to zero -- note destroys current_block contents */
	result=dos33_set_filesize(*new_inode,current_block,
			type&0x7f, 0);

	result=dos33_read_inode(*new_inode);

	return result;

}

	/* Give inode a name */
static int32_t dos33fs_link_inode(struct inode_type *inode,
				const char *name) {

	char catalog_block[DOS33_BLOCK_SIZE];
	char dos33_filename[DOS33_MAX_FILENAME_SIZE+1];
	int32_t cat_entry;
	uint32_t block_location,i;
	uint32_t next_t,next_s;

	if (debug) {
		printk("dos33fs: giving inode %x name %s\n",
			inode->number,name);
	}

	/* error if filename too long */
	if (strlen(name)>DOS33_MAX_FILENAME_SIZE) {
		return -E2BIG;
	}

	strncpy(dos33_filename,name,strlen(name));
	/* padded with ' ' */
	for(i=strlen(name);i<DOS33_MAX_FILENAME_SIZE;i++) {
		dos33_filename[i]=' ';
	}
	/* high bit set */
	for(i=0;i<DOS33_MAX_FILENAME_SIZE;i++) {
		dos33_filename[i]|=0x80;
	}

	next_t=(inode->number>>16)&0xff;
	next_s=(inode->number>>8)&0xff;
	cat_entry=(inode->number&0xff);

	block_location=ts(next_t,next_s);
	inode->sb->block->block_ops->read(inode->sb->block,
				block_location,DOS33_BLOCK_SIZE,catalog_block);

	for(i=0;i<DOS33_MAX_FILENAME_SIZE;i++) {
		catalog_block[DOS33_CAT_FIRST_ENTRY+
			(cat_entry*DOS33_CAT_ENTRY_SIZE)+
			DOS33_CAT_OFFSET_FILE_NAME+i]=dos33_filename[i];
	}

	inode->sb->block->block_ops->write(inode->sb->block,
				block_location,DOS33_BLOCK_SIZE,catalog_block);

	inode->hard_links++;

	return 0;

}





static void dos33fs_write_superblock(struct superblock_type *sb) {

	uint32_t vtoc_location = ts(17,0);	/* Usually at 17:0 */

	sb->block->block_ops->write(sb->block,
				vtoc_location,DOS33_VTOC_SIZE,sb->private);

	return;
}

static struct superblock_operations dos33fs_sb_ops = {
	.truncate_inode = dos33fs_truncate_inode,
	.write_inode = dos33fs_write_inode,
	.statfs = dos33fs_statfs,
	.lookup_inode = dos33fs_lookup_inode,
	.setup_fileops = dos33fs_setup_fileops,
	.write_superblock = dos33fs_write_superblock,
	.unlink_inode = dos33fs_unlink_inode,
	.destroy_inode = dos33fs_destroy_inode,
	.make_inode = dos33fs_make_inode,
	.link_inode = dos33fs_link_inode,
};




/* Note, should allocate one of these for each mounted fs */
static char vtoc_list[DOS33_VTOC_SIZE][DOS33_MAX_VTOCS];	/* 256 bytes */
static int32_t next_vtoc=0;

int32_t dos33fs_mount(struct superblock_type *sb,
					struct block_dev_type *block) {


	uint32_t vtoc_location = ts(17,0);	/* Usually at 17:0 */
	char *vtoc;

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

	/* Set block size */
	sb->block_size=DOS33_BLOCK_SIZE;

	/* Point to our superblock operations */
	sb->sb_ops=dos33fs_sb_ops;

	/* point to root dir of filesystem */
	/* we fake a "." directory entry */
	sb->root_dir=vtoc[DOS33_VTOC_FIRST_CAT_TRACK]<<16 |
			vtoc[DOS33_VTOC_FIRST_CAT_SECTOR]<<8 | 0xff;

	/* update free blocks */
	dos33_update_blocks_free(sb);

	if (debug) {
		printk("Mounted DOS33fs vol %d Tracks %d Sectors %d\n",
			vtoc[DOS33_VTOC_VOLUME],
			vtoc[DOS33_VTOC_NUM_TRACKS],
			vtoc[DOS33_VTOC_NUM_SECTORS]);
	}

	return 0;

}
