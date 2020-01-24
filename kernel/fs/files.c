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

static struct file_object file_objects[MAX_OPEN_FILES];

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


int32_t close_syscall(uint32_t fd) {

	int32_t result;

	result=file_object_free(NULL);

	return result;

}


/****************************************************/
/* open                                             */
/****************************************************/

int32_t open_syscall(const char *pathname, uint32_t flags, uint32_t mode) {

	int32_t result;
	struct inode_type inode;

	if (debug) {
		printk("### Trying to open %s\n",pathname);
	}

	result=get_inode(pathname,&inode);
	if (result<0) {
		return -ENOENT;
	}

	if (debug) {
		printk("\tFound inode %x\n",inode.number);
	}

	result=file_object_allocate(inode.number);
	if (result<0) {
		return result;
	}

	/* Set up the file_ops */
	inode.sb->sb_ops.setup_fileops(&file_objects[result]);

	if (debug) printk("### opened fd %d\n",result);

	return result;

}

int32_t read_syscall(uint32_t fd, void *buf, uint32_t count) {

	int32_t result;
	struct file_object *file;

	/* Hack for now */
	if (fd==0) {
		result=console_read(buf,count);
		return result;
	}

	if (fd>=MAX_OPEN_FILES) {
		return -ENFILE;
	}

	file=&file_objects[fd];

	if (file->valid==0) {
		printk("Attempting to read from unsupported fd %d\n",fd);
		return -EBADF;
	}

	if (debug) printk("Attempting to read %d bytes from fd %d into %x\n",count,fd,buf);

	result=file->file_ops->read(file_objects[fd].inode,
					buf,count,
					&file_objects[fd].file_offset);

	return result;
}

int32_t write_syscall(uint32_t fd, void *buf, uint32_t count) {

	int32_t result;
	struct file_object *file;

	/* Hack */
	if ((fd==1) || (fd==2)) {
		result = console_write(buf, count);
		return result;
	}

	if (fd>=MAX_OPEN_FILES) {
		return -ENFILE;
	}

	file=&file_objects[fd];

	if (file->valid==0) {
		printk("Attempting to write to unsupported fd %d\n",fd);
		return -EBADF;
	}

	result=file->file_ops->write(file_objects[fd].inode,
					buf,count,
					&file_objects[fd].file_offset);

	return result;
}

void file_objects_init(void) {
	int i;

	for(i=0;i<MAX_OPEN_FILES;i++) {
		file_objects[i].valid=0;
	}

	/* Special case 0/1/2 (stdin/stdout/stderr) */
	/* FIXME: actually hook them up to be proper file objects */
	file_objects[0].valid=1;
	file_objects[1].valid=1;
	file_objects[2].valid=1;

	return;
}

int32_t getdents_syscall(uint32_t fd,
			struct vmwos_dirent *dirp, uint32_t count) {

	int result;
	struct file_object *file;

	if (fd>=MAX_OPEN_FILES) {
		return -ENFILE;
	}

	file=&file_objects[fd];

	if (file->valid==0) {
		printk("Attempting to getdents from unsupported fd %d\n",fd);
		return -EBADF;
	}

	/* FIXME: check if it's a directory fd */
	if (debug) {
	}

	result=file->file_ops->getdents(file_objects[fd].inode,
					&(file_objects[fd].file_offset),
					dirp,count);
	return result;

}

/* Change current working directory */
int32_t chdir_syscall(const char *path) {

	int32_t result;
	struct inode_type inode;

	result=get_inode(path,&inode);
	if (result<0) {
		return -ENOENT;
	}

	if ((inode.mode&S_IFMT)!=S_IFDIR) {
		return -ENOTDIR;
	}

	current_proc[get_cpu()]->current_dir=inode.number;

	return 0;
}


/* Get name of current working directory */
char *getcwd_syscall(char *buf, size_t size) {

#if 0
	struct vmwos_stat stat_buf;

	int32_t inode,result;

	inode=current_proc[get_cpu()]->current_dir;

#endif
	strncpy(buf,"BROKEN",size);

	return buf;

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

	result=file_objects[fd].file_ops->llseek(&file_objects[fd],
							offset,whence);

	return result;
}
