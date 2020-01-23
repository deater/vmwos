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

//extern int32_t root_dir;

/* Split a filename into the path part and the actual name part */
const char *split_filename(const char *start_ptr, char *name,
			int len) {

	const char *ptr=start_ptr;
	char *out=name;
	int length=0;

	if (debug) {
		printk("Splitting %s\n",start_ptr);
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
	return ptr;
}

/* Starts at root directory */
/* searches for current component */
/* If directory, continues down */

int32_t get_inode(const char *pathname, struct inode_type *inode) {

	int32_t result;
	const char *ptr;
	char full_path[MAX_PATH_LEN];
	struct superblock_type *sb;

	sb=superblock_lookup(pathname);
	if (sb==NULL) {
		return -ENOENT;
	}

	/* start at root directory */
	inode->number=sb->root_dir;
	inode->sb=sb;

	/* expand path */
	if (pathname[0]=='/') {
		strncpy(full_path,pathname,MAX_PATH_LEN);
	}
	else {
		/* FIXME: prepend current working directory */
		snprintf(full_path,MAX_PATH_LEN,"%s/%s","/home",pathname);
	}

	/* point one past leading slash */
	ptr=full_path+1;

	sb->sb_ops.lookup_inode(inode,ptr);

	return result;
}

int32_t stat_syscall(const char *pathname, struct vmwos_stat *buf) {

	int32_t result;
	struct inode_type inode;

	if (debug) {
		printk("### Trying to stat %s\n",pathname);
	}

	result=get_inode(pathname,&inode);
	if (result<0) {
		return -ENOENT;
	}

	buf->st_dev=inode.device;
	buf->st_ino=inode.number;
	buf->st_mode=inode.mode;
	buf->st_nlink=inode.hard_links;
	buf->st_uid=inode.uid;
	buf->st_gid=inode.gid;
	buf->st_rdev=inode.rdev;
	buf->st_size=inode.size;
	buf->st_blksize=inode.blocksize;
	buf->st_blocks=inode.blocks;
	buf->st_atime=inode.atime;
	buf->st_mtime=inode.mtime;
	buf->st_ctime=inode.ctime;

	return result;
}

