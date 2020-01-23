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

#include "fs/romfs/romfs.h"

#include "processes/process.h"

static int debug=0;

extern int32_t root_dir;

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

	/* FIXME: search to see which filesystem we are in */
	/* deepest is # of /s? */

	/* start at root directory */
	inode->number=root_dir;

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

	result=romfs_lookup_inode(inode,ptr);

	return result;
}

#if 0
static int make_path_canonical(const char *pathname, char *canon_name) {

	char temp_path[MAX_PATH_LEN];
	int i,j,len,out_offset;
	int slashes[MAX_SUBDIR_DEPTH];
	int num_slashes=0;
	int total_dots=0;

	/* Make path absolute */

	if (pathname[0]=='/') {
		strncpy(temp_path,pathname,MAX_PATH_LEN);
	}
	else {
		/* FIXME */
		snprintf(temp_path,MAX_PATH_LEN,"%s/%s","/home",pathname);
	}

	/* remove all . and .. */

	len=strlen(temp_path);
	out_offset=0;
	num_slashes=0;

	i=0;
	while(1) {
		/* Handle slashes, remove duplicate slashes */
		if (temp_path[i]=='/') {
			canon_name[out_offset]=temp_path[i];
			slashes[num_slashes]=out_offset;
			num_slashes++;
			out_offset++;
			i++;
			while(temp_path[i]=='/') i++;

			/* Handle dots */
			if (temp_path[i]=='.') {
				total_dots=0;
				j=i;
				while(1) {
					if (temp_path[j]=='.') {
						total_dots++;
					}
					else {
						if (temp_path[j]!='/') {
							total_dots=0;
						}
						break;
					}
					j++;
				}
				printk("Total dots=%d\n",total_dots);
				/* . , current dir, skip */
				if (total_dots==1) {
					i+=total_dots+1;
				} else
				/* . , current dir, go down a dir */
				if (total_dots==2) {
					i+=total_dots+1;
					num_slashes--;
					out_offset=slashes[num_slashes-1];
				}
			}
		}



		canon_name[out_offset]=temp_path[i];
		out_offset++;
		i++;
		if (i>len) break;
	}
	(void)slashes;
	return 0;
}
#endif

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

