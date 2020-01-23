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

int32_t root_dir=0;

static struct file_object file_objects[MAX_OPEN_FILES];

struct file_object_operations file_ops= {
	romfs_read_file,	/* read() */
	romfs_write_file,	/* write() */
	llseek_generic,		/* llseek() */
	NULL,			/* ioctl() */
	NULL			/* open() */
};

int32_t file_object_free(struct file_object *file) {

	return -EBADF;
}

int32_t file_object_allocate(uint32_t inode) {

	int32_t index;

	if (debug) printk("Attempting to allocate fd for inode %x\n",inode);

	index=0;
	while(1) {
		if (file_objects[index].valid==0) {
			file_objects[index].valid=1;
			file_objects[index].inode=inode;
			file_objects[index].file_offset=0;
			file_objects[index].count=0;
			break;
		}
		index++;
		if (index>=MAX_OPEN_FILES) {
			index=-ENFILE;
			break;
		}
	}

	if (debug) printk("### Allocated file %d\n",index);

	return index;
}


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

int32_t close_syscall(uint32_t fd) {

	int32_t result;

	result=file_object_free(NULL);

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

/****************************************************/
/* open                                             */
/****************************************************/

int32_t open_syscall(const char *pathname, uint32_t flags, uint32_t mode) {

	int32_t result;
	int32_t inode;

	if (debug) {
		printk("### Trying to open %s\n",pathname);
	}

#if 0
	result=make_path_canonical(pathname,canon_name);
	if (result<0) {
		return -E2BIG;
	}

	if (debug) {
		printk("### Canonical name is %s\n",canon_name);
	}
#endif

	inode=get_inode(pathname);
	if (inode<0) {
		return -ENOENT;
	}

	result=file_object_allocate(inode);
	if (result<0) {
		return result;
	}

	if (debug) printk("### opened fd %d\n",result);

	return result;

}

int32_t read_syscall(uint32_t fd, void *buf, uint32_t count) {

	int32_t result;


	if (fd==0) {
		result=console_read(buf,count);
	}
	else if (fd>=MAX_OPEN_FILES) {
		return -ENFILE;
	}
	else if (file_objects[fd].valid==0) {
		printk("Attempting to read from unsupported fd %d\n",fd);
		result=-EBADF;
	}
	else {
		if (debug) printk("Attempting to read %d bytes from fd %d into %x\n",count,fd,buf);

		result=file_ops.read(file_objects[fd].inode,
					buf,count,
					&file_objects[fd].file_offset);

		/* Helder adjusts file_ptr for us */

//		if (result>0) {
//			file_objects[fd].file_ptr+=result;
//		}
	}
	return result;
}

int32_t write_syscall(uint32_t fd, void *buf, uint32_t count) {

	int32_t result;

	if (fd==2) {
		int i;
		char *string = (char *)buf;
		if (debug) {
			printk("Writing %d bytes, %d\n",count,string[count-1]);
			for(i=0;i<count;i++) {
				printk("%x ",string[i]);
			}
			printk("\n");
		}
	}

	if ((fd==1) || (fd==2)) {
		result = console_write(buf, count);
	}
	else {

		result=file_ops.write(file_objects[fd].inode,
					buf,count,
					&file_objects[fd].file_offset);
	}
	return result;
}

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

struct superblock_t superblock_table[8];

int32_t mount_syscall(const char *source, const char *target,
	const char *filesystemtype, uint32_t mountflags,
	const void *data) {

	int32_t result=0;

	if (!strncmp(filesystemtype,"romfs",5)) {
		result=romfs_mount(&superblock_table[0]);
		if (result>=0) {
			root_dir=result;
			result=0;
		}
	}
	else {
		result=-ENODEV;
	}

	return result;
}


void file_objects_init(void) {
	int i;

	for(i=0;i<MAX_OPEN_FILES;i++) {
		file_objects[i].valid=0;
	}

	/* Special case 0/1/2 (stdin/stdout/stderr) */
	/* FIXME: actually hook them up to be proper fds */
	file_objects[0].valid=1;
	file_objects[1].valid=1;
	file_objects[2].valid=1;

	return;
}

int32_t getdents_syscall(uint32_t fd,
			struct vmwos_dirent *dirp, uint32_t count) {

	int result;

	if (fd>=MAX_OPEN_FILES) {
		return -ENFILE;
	}
	else if (file_objects[fd].valid==0) {
		printk("Attempting to getdents from unsupported fd %d\n",fd);
		result=-EBADF;
	}
	/* FIXME: check if it's a directory fd */
	else {
		if (debug) {
		}

		result=	romfs_getdents(file_objects[fd].inode,
					&(file_objects[fd].file_offset),
					dirp,count);
	}
	return result;

}

/* Change current working directory */
int32_t chdir_syscall(const char *path) {

	int32_t inode,result;

	struct vmwos_stat buf;

	inode=get_inode(path);
	if (inode<0) {
		return -ENOENT;
	}

	result=romfs_stat(inode, &buf);
	if (result<0) {
		return result;
	}

	if ((buf.st_mode&S_IFMT)!=S_IFDIR) {
		return -ENOTDIR;
	}

	current_proc[get_cpu()]->current_dir=inode;

	return 0;
}


/* Get name of current working directory */
char *getcwd_syscall(char *buf, size_t size) {

	struct vmwos_stat stat_buf;

	int32_t inode,result;

	inode=current_proc[get_cpu()]->current_dir;

	result=romfs_stat(inode, &stat_buf);

	(void)result;

	strncpy(buf,"BROKEN",size);

	return buf;

}

int32_t statfs_syscall(const char *path, struct vmwos_statfs *buf) {
	/* FIXME: lookup path */

	return romfs_statfs(&superblock_table[0],buf);
}


int64_t llseek_generic(struct file_object *file,
		int64_t offset, int32_t whence) {

	if (file==NULL) return -EBADF;

	switch(whence) {
		case SEEK_SET:
			file->file_offset=offset;
			break;
		case SEEK_CUR:
			file->file_offset+=offset;
			break;
		case SEEK_END:
			/* FIXME: need to get size from inode */
			return -ENOSYS;
			break;
		default:
			return -EINVAL;
	}

	return file->file_offset;
}


int64_t llseek_syscall(uint32_t fd, int64_t offset, int32_t whence) {

	int64_t result;

	result=file_ops.llseek(&file_objects[fd],offset,whence);

	return result;
}
