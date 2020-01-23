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

static int32_t root_dir=0;


/* Split a filename into the path part and the actual name part */
static const char *split_filename(const char *start_ptr, char *name,
			int len) {

	const char *ptr=start_ptr;
	char *out=name;
	int length=0;

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

int32_t get_inode(const char *pathname) {

	int32_t inode;
	char name[MAX_FILENAME_SIZE];
	const char *ptr=pathname;
	int32_t dir_inode;

	/* start at root directory */
	if (*ptr=='/') {
		dir_inode=root_dir;
		ptr++;
	}
	else {
		dir_inode=current_proc[get_cpu()]->current_dir;
	}

	if (*ptr==0) {
		return dir_inode;
	}

	while(1) {
		if (debug) {
			printk("get_inode: about to split %s\n",ptr);
		}

		ptr=split_filename(ptr,name,MAX_FILENAME_SIZE);

		if (debug) {
			printk("get_inode: di=%x path_part %s\n",
							dir_inode,name);
		}

		if (ptr==NULL) break;
		dir_inode=romfs_get_inode(dir_inode,name);
	}

	inode=romfs_get_inode(dir_inode,name);
	if (inode<0) {
		if (debug) printk("get_inode: error opening %s\n",name);
	}

	return inode;
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

	int32_t inode;
	int32_t result;

	if (debug) {
		printk("### Trying to stat %s\n",pathname);
	}

	inode=get_inode(pathname);
	if (inode<0) {
		return -ENOENT;
	}

	result=romfs_stat(inode, buf);

	return result;
}

