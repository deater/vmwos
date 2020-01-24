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

	/* FIXME */
	return &superblock_table[0];
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

	sb=superblock_allocate();
	if (sb==NULL) {
		return -ERANGE;
	}

	/* FIXME: setup a data structure to search all compiled-in fses */

	if (!strncmp(filesystemtype,"romfs",5)) {
		result=romfs_mount(sb);
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
		sb->size/1024,filesystemtype,target);

	return result;
}

/*************************************************/
/* statfs()                                      */
/*************************************************/


int32_t statfs_syscall(const char *path, struct vmwos_statfs *buf) {

	struct inode_type inode;
	int32_t result;

	if (debug) printk("### statfs on \"%s\"\n",path);

	result=get_inode(path,&inode);
	if (result<0) {
		if (debug) printk("\tget_inode result %d\n",result);
		return result;
	}

	if (debug) printk("\tgot inode %x of %s\n",
		inode.number,inode.sb->mountpoint);

	return inode.sb->sb_ops.statfs(inode.sb,buf);

}
