#include <stdint.h>
#include <stddef.h>

#include "lib/errors.h"
#include "lib/printk.h"
#include "lib/string.h"
#include "lib/smp.h"

#include "fs/files.h"
#include "fs/inodes.h"
#include "fs/superblock.h"

#include "fs/romfs/romfs.h"
#include "fs/dos33fs/dos33fs.h"

#include "drivers/block.h"

static int debug=0;

struct superblock_type superblock_table[MAX_MOUNTS];

/*************************************************/
/* superblock_allocate()                         */
/*************************************************/

struct superblock_type *superblock_allocate(void) {

	int32_t i;

	for(i=0;i<MAX_MOUNTS;i++) {
		if (superblock_table[i].valid==0) {
			superblock_table[i].valid=1;
			return &superblock_table[i];
		}
	}
	return NULL;
}

/*************************************************/
/* superblock_lookup()                           */
/*************************************************/

struct superblock_type *superblock_lookup(const char *path) {

	int32_t i,which_root=0;
	struct superblock_type *sb_root=NULL;

	/* find root */
	for(i=0;i<MAX_MOUNTS;i++) {
		if (superblock_table[i].valid) {
			if (!strncmp("/",
				superblock_table[i].mountpoint,
				strlen(superblock_table[i].mountpoint))) {
				sb_root=&superblock_table[i];
				which_root=i;
				break;
			}
		}
	}

	/* find other mountpoint */
	for(i=0;i<MAX_MOUNTS;i++) {
		if (i==which_root) continue;
		if (superblock_table[i].valid) {
			if (!strncmp(
				superblock_table[i].mountpoint,
				path,
				strlen(superblock_table[i].mountpoint))) {
				return &superblock_table[i];
			}
		}
	}
	return sb_root;

	/* return root path */


}


/*************************************************/
/*************************************************/
/*************************************************/

/*************************************************/
/* mount()                                       */
/*************************************************/


int32_t mount_syscall(const char *source, const char *target,
	const char *filesystemtype, uint32_t mountflags,
	const void *data) {

	int32_t result=0;
	struct superblock_type *sb;
	struct block_dev_type *block;

	sb=superblock_allocate();
	if (sb==NULL) {
		printk("Unable to allocate superblock\n");
		return -ERANGE;
	}

	block=block_dev_find(source);
	if (block==NULL) {
		return -ENODEV;
	}

	/* FIXME: setup a data structure to search all compiled-in fses */

	if (!strncmp(filesystemtype,"romfs",5)) {
		result=romfs_mount(sb,block);
		if (result<0) {
			return -EINVAL;
		}
		//root_dir=sb->root_dir;
		if (debug) printk("ROOT_DIR=%x\n",sb->root_dir);
		result=0;
	}
	else if (!strncmp(filesystemtype,"dos33fs",5)) {
		result=dos33fs_mount(sb,block);
		if (result<0) {
			return -EINVAL;
		}
		//root_dir=sb->root_dir;
		if (debug) printk("ROOT_DIR=%x\n",sb->root_dir);
		result=0;
	}
	else {
		result=-ENODEV;
	}

	strncpy(sb->mountpoint,target,MAX_FILENAME_SIZE);

	printk("Mounted %dK %s filesystem at %s\n",
		sb->blocks*sb->block_size/1024,filesystemtype,target);

	return result;
}

/*************************************************/
/* statfs()                                      */
/*************************************************/


int32_t statfs_syscall(const char *path, struct vmwos_statfs *buf) {

	struct inode_type *inode;
	int32_t result;

	if (debug) printk("### statfs on \"%s\"\n",path);

	result=inode_lookup_and_alloc(path,&inode);
	if (result<0) {
		if (debug) printk("\tget_inode result %d\n",result);
		return result;
	}

	if (debug) {
		printk("\tgot inode %x of %s\n",
			inode->number,inode->sb->mountpoint);
	}

	result=inode->sb->sb_ops.statfs(inode->sb,buf);

	inode_free(inode);

	return result;
}
