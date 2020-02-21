#include <stdint.h>
#include <stddef.h>

#include "lib/errors.h"
#include "lib/printk.h"
#include "lib/string.h"
#include "lib/memset.h"
#include "lib/smp.h"

#include "drivers/console/console_io.h"

#include "fs/files.h"
#include "fs/inodes.h"
#include "fs/superblock.h"

#include "processes/process.h"

static int debug=0;

static struct inode_type inodes[NUM_INODES];


/* Split a filename into the path part and the actual name part */
/* Returns pointer to the last '/' in the file + 1 */
/* Also the last '/' is turned to a 0 */
const char *split_pathname(char *fullpath,int len) {

	int ptr;

	ptr=strlen(fullpath);

	while(1) {
		if (ptr==0) break;
		if (fullpath[ptr]=='/') {
			fullpath[ptr]=0;
			return &fullpath[ptr+1];
		}
		ptr--;
	}

	return NULL;
}



/* start_ptr ??? */
/* name should be an already allocated buffer */
/* len is the length */
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


struct inode_type *inode_allocate(void) {

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

static struct inode_type *inode_find_existing(
		struct superblock_type *sb,
		int32_t inode_number) {

	int i;

	/* find an inode */
	for(i=0;i<NUM_INODES;i++) {
		if (inodes[i].sb==sb) {
			if (inodes[i].number==inode_number) {
				if (debug) {
					printk("Found existing inode %x\n",
						inode_number);
				}
				return &inodes[i];
			}

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

	if (debug) {
		printk("ilac: looking up %s, inodes[0].count=%d\n",
			pathname,inodes[0].count);
	}

	temp_inode=inode_allocate();
	if (temp_inode==NULL) {
		if (debug) {
			printk("ilac: no inodes avail!\n");
		}
		return -ENOMEM;
	}

	if (debug) {
		printk("ilac: got an inode %p count=%d\n",
					temp_inode,temp_inode->count);
	}

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

	if (debug) {
		printk("ilac: looking up sb for %s\n",full_path);
	}

	/* Look to see which mountpoint we are in */
	sb=superblock_lookup(full_path);
	if (sb==NULL) {
		return -ENOENT;
	}

	/* start at root directory of mountpoint */
	temp_inode->number=sb->root_dir;
	temp_inode->sb=sb;

	/* point past mountpoint */
	ptr=full_path;
	ptr+=strlen(sb->mountpoint);

	/* skip any leading /s */
	while((*ptr=='/')&&(*ptr!='\0')) ptr++;

	result=sb->sb_ops.lookup_inode(temp_inode,ptr);

	if (result<0) {
		inode_free(temp_inode);
		return -ENOENT;
	}

	*inode=inode_find_existing(temp_inode->sb,temp_inode->number);
	if (*inode==NULL) {
		*inode=temp_inode;
	}
	else {
		(*inode)->count++;
		inode_free(temp_inode);
	}
	return result;
}

int32_t inode_free(struct inode_type *inode) {

	/* FIXME: locking */

	if (debug) {
		printk("inode_free: before %p count=%d\n",inode,inode->count);
	}

	if (inode->count==0) {
		printk("ERROR: Attempting to free already freed inode!\n");
		return -1;
	}

	if (inode->count) {
		inode->count--;
	}

	if ((inode->count==0 ) && (inode->hard_links==0)) {
		if (debug) {
			printk("attempting to destroy inode %x\n",
				inode->number);
		}
		inode->sb->sb_ops.destroy_inode(inode);
	}

	/* clear out the inode with invalid data */
	if (inode->count==0 ) {
		memset(inode,'V',sizeof(struct inode_type));
		inode->count=0;
	}

	return 0;
}

/****************************************************/
/* stat syscall                                     */
/****************************************************/

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

/****************************************************/
/* chmod syscall                                    */
/****************************************************/

int32_t chmod_syscall(const char *pathname, int32_t mode) {

	int32_t result;
	struct inode_type *inode;

	if (debug) {
		printk("### Trying to chmod %s\n",pathname);
	}

	result=inode_lookup_and_alloc(pathname,&inode);
	if (result<0) {
		return -ENOENT;
	}

	/* FIXME: lots of checks */

	/* actually update to disk */
	inode->mode=(inode->mode&0xfffffe00)|(mode&0x1ff);
	inode->sb->sb_ops.write_inode(inode);

	inode_free(inode);

	return result;
}

int32_t truncate_inode(struct inode_type *inode, int64_t size) {

	int32_t result;

	if (debug) {
		printk("Truncating inode %x to %lld\n",inode->number,size);
	}

	/* FIXME: check permissions */

	/* tell filesystem to truncate inode */
	result=inode->sb->sb_ops.truncate_inode(inode,size);

	if (result>=0) {
		/* FIXME: Update the ctime/mtime */

		/* write inode back to disk */
		if (debug) {
			printk("truncate: writing inode %x back to disk\n",
				inode->number);
		}
		inode->sb->sb_ops.write_inode(inode);
	}

	return result;
}

/****************************************************/
/* truncate64 syscall                               */
/****************************************************/

int32_t truncate64_syscall(const char *pathname, uint64_t size) {

	int32_t result;
	struct inode_type *inode;

	if (debug) {
		printk("truncate64: truncating %s to %lld\n",pathname,size);
	}

	result=inode_lookup_and_alloc(pathname,&inode);
        if (result<0) {
                return -ENOENT;
        }

	if (debug) {
		printk("truncate64: found inode %x\n",inode->number);
	}

	result=truncate_inode(inode,size);

	if (debug) {
		printk("truncate64: result %d\n",result);
	}

	inode_free(inode);

	return result;
}


/****************************************************/
/* unlink syscall                                   */
/****************************************************/

int32_t unlink_syscall(const char *pathname) {

	int32_t result;
	struct inode_type *inode;

	if (debug) {
		printk("unlink: attempting to unlink %s\n",pathname);
	}

	result=inode_lookup_and_alloc(pathname,&inode);
        if (result<0) {
                return -ENOENT;
        }

	if (debug) {
		printk("unlink: found inode %x\n",inode->number);
	}

	if (inode->mode&S_IFDIR) {
		result=-EISDIR;
		goto unlink_syscall_free_and_return;
	}

	result=inode->sb->sb_ops.unlink_inode(inode);
	if (result>=0) {
		inode->hard_links--;
	}

unlink_syscall_free_and_return:

	inode_free(inode);

	return result;

}
