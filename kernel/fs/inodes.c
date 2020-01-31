#include <stdint.h>
#include <stddef.h>

#include "lib/errors.h"
#include "lib/printk.h"
#include "lib/string.h"
#include "lib/smp.h"

#include "drivers/console/console_io.h"

#include "fs/files.h"
#include "fs/inodes.h"
#include "fs/superblock.h"

#include "processes/process.h"

static int debug=0;

static struct inode_type inodes[NUM_INODES];

/* Split a filename into the path part and the actual name part */
const char *split_filename(const char *start_ptr, char *name,
			int len) {

	const char *ptr=start_ptr;
	char *out=name;
	int length=0;

	if (debug) {
		printk("Splitting \"%s\"\n",start_ptr);
	}
	while(1) {
		if (*ptr==0) {
			*out=0;
			return NULL;
		}

		if (length>=(len-1)) {
			*out=0;
			return NULL;
		}

		if (*ptr=='/') {
			*out=0;
			ptr++;
			break;
		}
		*out=*ptr;
		ptr++;
		out++;
		length++;
	}
	if (debug) printk("Got %s\n",ptr);

	return ptr;
}

static struct inode_type *inode_allocate(void) {

	int i;

	/* FIXME: locking */

	/* FIXME: search for existing first */

	/* allocate an inode */
	for(i=0;i<NUM_INODES;i++) {
		if (inodes[i].count==0) {
			if (debug) printk("Allocated inode %d (%p) count=%d\n",
						i,&inodes[i],
						inodes[i].count);
			inodes[i].count=1;
			return &inodes[i];
		}
	}
	return NULL;
}


/* Starts at root directory */
/* searches for current component */
/* If directory, continues down */

int32_t inode_lookup_and_alloc(const char *pathname,
					struct inode_type **inode) {

	int32_t result;
	const char *ptr;
	char full_path[MAX_PATH_LEN];
	struct superblock_type *sb;
	struct inode_type *temp_inode;

	if (debug) printk("ilac: inodes[0].count=%d\n",inodes[0].count);

	temp_inode=inode_allocate();
	if (temp_inode==NULL) {
		return -ENOMEM;
	}

	if (debug) printk("ilac: got an inode %p count=%d\n",
					temp_inode,temp_inode->count);

	/* expand path */
	if (pathname[0]=='/') {
		strncpy(full_path,pathname,MAX_PATH_LEN);
	}
	else {
		/* prepend current working directory */
		snprintf(full_path,MAX_PATH_LEN,"%s/%s",
			current_proc[get_cpu()]->current_dir,
			pathname);
	}

	/* Look to see which mountpoint we are in */
	sb=superblock_lookup(pathname);
	if (sb==NULL) {
		return -ENOENT;
	}

	/* start at root directory */
	temp_inode->number=sb->root_dir;
	temp_inode->sb=sb;

	/* point past mountpoint */
	ptr=full_path;
	ptr+=strlen(sb->mountpoint);

//	ptr=full_path;
//	while((*ptr=='/')&&(*ptr!='\0')) ptr++;

	result=sb->sb_ops.lookup_inode(temp_inode,ptr);

	if (result<0) {
		inode_free(temp_inode);
		return -ENOENT;
	}

	*inode=temp_inode;

	return result;
}

int32_t inode_free(struct inode_type *inode) {

	/* FIXME: locking */

	if (debug) printk("Freeing inode %p count=%d\n",inode,inode->count);

	if (inode->count) inode->count--;

	return 0;
}

int32_t stat_syscall(const char *pathname, struct vmwos_stat *buf) {

	int32_t result;
	struct inode_type *inode;

	if (debug) {
		printk("### Trying to stat %s\n",pathname);
	}

	result=inode_lookup_and_alloc(pathname,&inode);
	if (result<0) {
		return -ENOENT;
	}

	buf->st_dev=inode->device;
	buf->st_ino=inode->number;
	buf->st_mode=inode->mode;
	buf->st_nlink=inode->hard_links;
	buf->st_uid=inode->uid;
	buf->st_gid=inode->gid;
	buf->st_rdev=inode->rdev;
	buf->st_size=inode->size;
	buf->st_blksize=inode->blocksize;
	buf->st_blocks=inode->blocks;
	buf->st_atime=inode->atime;
	buf->st_mtime=inode->mtime;
	buf->st_ctime=inode->ctime;

	inode_free(inode);

	return result;
}

