#include <stdint.h>
#include <stddef.h>

#include "lib/errors.h"
#include "lib/printk.h"
#include "lib/string.h"
#include "lib/smp.h"

#include "fs/files.h"
#include "fs/inodes.h"
#include "fs/superblock.h"

#include "drivers/console/console_io.h"
#include "drivers/char.h"
#include "drivers/block.h"

#include "processes/process.h"

static int debug=0;

static struct file_object file_objects[MAX_OPEN_FILES];

void files_increment_count(struct file_object *file) {
	file->count++;
	file->inode->count++;
}

int32_t file_object_free(struct file_object *file) {

	/* FIXME: locking */

	if (debug) {
		printk("Attempting to free file object %p\n",file);
	}

	/* Free the corresponding inode */
	if (file->inode) inode_free(file->inode);

	/* Reduce the count */
	if (file->count) file->count--;

	if (debug) {
		if (file->count==0) {
			printk("File object %p completely free\n",file);
		}
	}

	return 0;
}

int32_t file_object_allocate(struct inode_type *inode) {

	int32_t index;

	if (debug) {
		printk("Attempting to allocate file object for inode %x\n",
			inode->number);
	}

	index=0;
	while(1) {
		if (file_objects[index].count==0) {
			file_objects[index].count=1;
			file_objects[index].inode=inode;
			file_objects[index].file_offset=0;
			break;
		}
		index++;
		if (index>=MAX_OPEN_FILES) {
			index=-ENFILE;
			break;
		}
	}

	if (debug) {
		printk("### Allocated file object %d (%p)\n",
			index,&file_objects[index]);
	}

	return index;
}

/* Map user-program fd to the file_object struct */
static int32_t map_fd_to_file(uint32_t fd, struct file_object **file) {

	if (fd<0) {
		return -ENFILE;
	}

	if (fd>=MAX_FD_PER_PROC) {
		return -ENFILE;
	}

	*file=current_proc[get_cpu()]->files[fd];
	if (*file==NULL) {
		return -ENFILE;
	}

	if ((*file)->count==0) {
		printk("ERROR: Attempting to read from uknown fd %d\n",fd);
		return -EBADF;
	}

	if (debug) {
		printk("Opening fd %d, found %p\n",
			fd,*file);
	}

	return 0;

}

/****************************************************/
/* close syscall                                    */
/****************************************************/

int32_t close_syscall(uint32_t fd) {

	int32_t result;
	struct file_object *file;

	result=map_fd_to_file(fd, &file);
	if (result<0) {
		return result;
	}

	result=file_object_free(file);

	current_proc[get_cpu()]->files[fd]=NULL;

	if (debug) {
		printk("Closing fd %d, file %p\n",fd,file);
	}

	return result;

}

struct inode_type *file_get_inode(int32_t which) {

	return file_objects[which].inode;
}

static int32_t create_file_object(const char *pathname,
					struct inode_type **inode) {

	char full_path[MAX_FILENAME_SIZE];
	const char *filename,*directory;
	struct inode_type *dir_inode;
	int32_t result;

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
		printk("create: trying to create file %s (expanded to %s)\n",
			pathname,full_path);
	}

	/* split up the filename/pathname */
	filename=split_pathname(full_path,MAX_FILENAME_SIZE);
	directory=full_path;

	if (debug) {
		printk("create: trying to create file %s in directory %s\n",
			filename,directory);
	}

	/* Open directory that we want */
	result=inode_lookup_and_alloc(directory,&dir_inode);
	if (result<0) {
		return result;
	}

	*inode=inode_allocate();
	if (*inode==NULL) {
		printk("cfo trouble allocating inode\n");
		return -ENOMEM;
	}

	if (debug) {
		printk("making inode in dir %x\n",dir_inode->number);
	}

	result=dir_inode->sb->sb_ops.make_inode(dir_inode,inode);
	if (result<0) {
		printk("trouble making inode in dir %x\n",
			dir_inode->number);
		return result;
	}

//	result=dir_inode->sb->sb_ops.read_inode(dir_inode,inode);
//	if (result<0) {
//		return result;
//	}

	if (debug) {
		printk("linking inode %x as %s\n",(*inode)->number,filename);
	}
	result=dir_inode->sb->sb_ops.link_inode(*inode,filename);
	if (result<0) {
		printk("trouble linking %s to inode %x\n",
			filename,(*inode)->number);
		return result;
	}

	inode_free(dir_inode);

	return 0;
}

/****************************************************/
/* open file object                                 */
/****************************************************/

int32_t open_file_object(
	struct file_object **file,
	const char *pathname, uint32_t flags, uint32_t mode) {

	int32_t result;
	struct inode_type *inode;

	if (debug) {
		printk("### Trying to open %s\n",pathname);
	}

	/* Lookup existing inode */
	result=inode_lookup_and_alloc(pathname,&inode);
	if (result<0) {

		/* File doesn't exist */
		/* Want to create new file? */
		if (flags&O_CREAT) {
			result=create_file_object(pathname,&inode);
			if (result<0) {
				return result;
			}
		}
		else {
			return -ENOENT;
		}
	}

	if (debug) {
		printk("\tFound inode %x\n",inode->number);
	}

	result=file_object_allocate(inode);
	if (result<0) {
		return result;
	}

	*file=&file_objects[result];

	(*file)->flags=flags;

	/* Set up the file_ops */
	inode->sb->sb_ops.setup_fileops(*file);

	/* If O_TRUNC then truncate file */
	if (flags&O_TRUNC) {
		inode->sb->sb_ops.truncate_inode(inode,0);
		(*file)->file_offset=0;
	}

	/* If O_APPEND then set position to end of file */
	if (flags&O_APPEND) {
		(*file)->file_offset=inode->size;
	}

	if (debug) printk("### opened fd %d\n",result);

	return 0;
}


/****************************************************/
/* open syscall                                     */
/****************************************************/

int32_t open_syscall(const char *pathname, uint32_t flags, uint32_t mode) {

	int32_t result,i;
	struct file_object *file;

	/* need to map this to a per-process file descriptor */
	for(i=0;i<MAX_FD_PER_PROC;i++) {
		if (current_proc[get_cpu()]->files[i]==NULL) {
			break;
		}
	}
	if (i==MAX_FD_PER_PROC) {
		return -ENFILE;
	}

	/* result is which file_object[] */
	result=open_file_object(&file,pathname,flags,mode);

	if (result<0) {
		return result;
	}
	else {
		current_proc[get_cpu()]->files[i]=file;
		return i;
	}
}


/****************************************************/
/* read syscall                                     */
/****************************************************/

int32_t read_syscall(uint32_t fd, void *buf, uint32_t count) {

	int32_t result,mode;
	struct file_object *file;
	struct char_dev_type *char_dev;
	struct block_dev_type *block_dev;

	result=map_fd_to_file(fd,&file);
	if (result<0) {
		return result;
	}

	/* Handle stdin: Hack for now */
//	if (fd==0) {
//		result=console_read(buf,count,nonblock);
//		return result;
//	}


	/* If trying to read a write-only file... */
	if ((file->flags&O_RW_MASK) == O_WRONLY) {
		return -EBADF;
	}

	if (debug) {
		printk("Attempting to read %d bytes from fd %d into %x\n",
			count,fd,buf);
	}

	mode=(file->inode->mode & S_IFMT);

	if (mode==S_IFBLK) {
		block_dev=block_dev_lookup(file->inode->device);
		result=block_dev->block_ops->read(block_dev,
					file->file_offset,count,buf);
		if (result>=0) file->file_offset+=result;
	}
	else if (mode==S_IFCHR ) {
		char_dev=char_dev_lookup(file->inode->device);
		result=char_dev->char_ops->read(file,buf,count);
	}
	else if (mode==S_IFREG) {

		result=file->file_ops->read(
				file->inode,
				buf,count,
				&(file->file_offset));
	} else if (mode==S_IFDIR) {
		return -EISDIR;
	}
	else {
		printk("read: unknown inode type\n");
		return -EINVAL;
	}

	return result;
}

/****************************************************/
/* write syscall                                    */
/****************************************************/

int32_t write_syscall(uint32_t fd, void *buf, uint32_t count) {

	int32_t result,mode;
	struct file_object *file;
	struct char_dev_type *char_dev;
	struct block_dev_type *block_dev;

	if (debug) {
		printk("Attempting to write to fd %d\n",fd);
	}

	/* Hack */
//	if ((fd==1) || (fd==2)) {
//		result = console_write(buf, count);
//		return result;
//	}

	result=map_fd_to_file(fd,&file);
	if (result<0) {
		return result;
	}

	/* Check permissions */

	/* If trying to write a read-only file... */
	if ((file->flags&O_RW_MASK) == O_RDONLY) {
		//printk("Attempting to write a read only file %d\n",fd);
		return -EBADF;
	}

	mode=(file->inode->mode & S_IFMT);

	if (mode==S_IFBLK) {
		//printk("Attempting to write a block device\n");
		block_dev=block_dev_lookup(file->inode->device);
		result=block_dev->block_ops->write(block_dev,
					file->file_offset,count,buf);
	}
	else if (mode==S_IFCHR ) {
		//printk("Attempting to write a char device\n");
		char_dev=char_dev_lookup(file->inode->device);
		result=char_dev->char_ops->write(file,buf,count);
	}
	else if (mode==S_IFREG) {
		//printk("Attempting to write a normal file\n");
		result=file->file_ops->write(file->inode,
					buf,count,
					&(file->file_offset));
	} else if (mode==S_IFDIR) {
		return -EISDIR;
	}
	else {
		printk("write: unknown inode type\n");
		return -EINVAL;
	}

	return result;
}

void file_objects_init(void) {
	int i;

	for(i=0;i<MAX_OPEN_FILES;i++) {
		file_objects[i].count=0;
	}

	return;
}

/****************************************************/
/* getdents syscall                                 */
/****************************************************/

int32_t getdents_syscall(uint32_t fd,
			struct vmwos_dirent *dirp, uint32_t count) {

	int result;
	struct file_object *file;

	if (fd>=MAX_OPEN_FILES) {
		return -ENFILE;
	}

	file=&file_objects[fd];

	if (file->count==0) {
		printk("Attempting to getdents from unknown fd %d\n",fd);
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

/****************************************************/
/* chdir syscall                                    */
/****************************************************/

/* Change current working directory */
/* FIXME: should make path canonical somehow */
int32_t chdir_syscall(const char *path) {

	int32_t result,len;
	struct inode_type *inode;
	char new_path[MAX_PATH_LEN];

	/* See if too big */
	len=strlen(path);
	if (len>MAX_PATH_LEN) {
		return -E2BIG;
	}

	/* expand path */

	/* Absolute */
        if (path[0]=='/') {
                strncpy(new_path,path,MAX_PATH_LEN);
        }
	/* Relative */
        else {
                /* prepend old path */
                snprintf(new_path,MAX_PATH_LEN,"%s/%s",
                        current_proc[get_cpu()]->current_dir,
                        path);
        }

	/* Strip off trailing slashes if not in root */
	if ((len>1) && (new_path[len-1]=='/')) {
		len--;
		new_path[len]='\0';
	}

	/* Check to see if destination exists */
	result=inode_lookup_and_alloc(new_path,&inode);
	if (result<0) {
		return -ENOENT;
	}

	if ((inode->mode&S_IFMT)!=S_IFDIR) {
		inode_free(inode);
		return -ENOTDIR;
	}

	/* Things check out, copy over old path */
	strncpy(current_proc[get_cpu()]->current_dir,new_path,MAX_PATH_LEN);

	/* TODO: keep a pointer to the inode in the current dir? */
	inode_free(inode);
	return 0;
}

/****************************************************/
/* getcwd syscall                                   */
/****************************************************/

/* Get name of current working directory */
char *getcwd_syscall(char *buf, size_t size) {


	strncpy(buf,current_proc[get_cpu()]->current_dir,size);

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

/****************************************************/
/* llseek syscall                                   */
/****************************************************/

int64_t llseek_syscall(uint32_t fd, int64_t offset, int32_t whence) {

	int64_t result;
	struct file_object *file;
	int32_t mode;

	result=map_fd_to_file(fd, &file);
	if (result<0) {
		return result;
	}

	mode=(file->inode->mode & S_IFMT);

	if (mode==S_IFBLK) {
		result=llseek_generic(file,offset,whence);
	}
	else if (mode==S_IFCHR ) {
		result=0;
	}
	else if (mode==S_IFREG) {
		result=file->file_ops->llseek(file,offset,whence);
	}
	else if (mode==S_IFDIR) {
		result=-EISDIR;
	}
	else {
		result=-EINVAL;
	}

	return result;
}

/* setup stdin/stdout/stderr */
struct file_object *file_special(int which) {

	struct file_object *file;
	struct inode_type *inode;

	/* set up inode for it */
	inode=inode_allocate();
	inode->number=which;
	inode->count=1;
	inode->hard_links=1;
	inode->uid=0;
	inode->gid=0;
	inode->device=(CONSOLE_MAJOR<<16);
	inode->mode=S_IFCHR | 0666;

	file=&file_objects[which];
	file->count=1;

	file->inode=inode;



	/* stdin */
	if (which==0) {
		file->flags=O_RDONLY;
	}

	/* stdout */
	if (which==1) {
		file->flags=O_WRONLY;
	}

	/* stderr */
	if (which==2) {
		file->flags=O_WRONLY;
	}

	return file;

}

/****************************************************/
/* ftruncate64 syscall                              */
/****************************************************/

int32_t ftruncate64_syscall(int32_t fd, uint64_t size) {

	int32_t result;
	struct file_object *file;

	if (debug) {
		printk("ftruncate64: truncating fd %d to size %lld\n",fd,size);
	}

	result=map_fd_to_file(fd,&file);
	if (result<0) {
		return result;
	}

	/* If trying to truncate a read-only file... */
	if ((file->flags&O_RW_MASK) == O_RDONLY) {
		return -EBADF;
	}

	result=truncate_inode(file->inode,size);

	if (debug) {
		printk("ftruncate64: result %d\n",result);
	}

	return result;
}

/****************************************************/
/* fcntl syscall                                    */
/****************************************************/

int32_t fcntl_syscall(uint32_t fd, int32_t cmd, uint32_t third) {

	int32_t result;
	struct file_object *file;

	if (debug) {
		printk("fcntl: on fd %d with %x %x\n",fd,cmd,third);
	}

	result=map_fd_to_file(fd,&file);
	if (result<0) {
		return result;
	}

	switch(cmd) {

		case F_GETFL:
			return file->flags;
			break;
		case F_SETFL:
			file->flags=third;
			result=0;
			break;

		default:
			result=-ENOSYS;
			break;
	}

	return result;
}

/****************************************************/
/* ioctl syscall                                    */
/****************************************************/

int32_t ioctl_syscall(uint32_t fd, int32_t cmd,
					uint32_t third, uint32_t fourth) {

	int32_t result,mode;
	struct file_object *file;
	struct char_dev_type *char_dev;
	struct block_dev_type *block_dev;

	if (debug) {
		printk("ioctl: on fd %d with %x %x %x\n",fd,cmd,third,fourth);
	}

	result=map_fd_to_file(fd,&file);
	if (result<0) {
		return result;
	}


	mode=(file->inode->mode & S_IFMT);

	if (mode==S_IFBLK) {
		//printk("Attempting to ioctl a block device\n");
		block_dev=block_dev_lookup(file->inode->device);
		result=block_dev->block_ops->ioctl(block_dev,cmd,third,fourth);

	}
	else if (mode==S_IFCHR ) {
		//printk("Attempting to ioctl a char device\n");
		char_dev=char_dev_lookup(file->inode->device);
		result=char_dev->char_ops->ioctl(file,cmd,third,fourth);
	}
	else {
		return -ENOTTY;
	}

	return result;
}

/****************************************************/
/* dup2 syscall                                     */
/****************************************************/

int32_t dup2_syscall(uint32_t oldfd, int32_t newfd) {

	int32_t result;
	struct file_object *file_old,*file_new;

	/* This is a tricky one */
	/* If newfd is not being used by process, point it to oldfd */
	/* If newfd is being used, close it, then point to oldfd */

	if (debug) {
		printk("dup2: old %d new %d\n",oldfd,newfd);
	}

	if (current_proc[get_cpu()]->files[newfd]==NULL) {

	}
	else {
		/* close old file */
		result=map_fd_to_file(newfd, &file_new);
		if (result<0) {
			return result;
		}

		result=file_object_free(file_new);
	}

	result=map_fd_to_file(oldfd,&file_old);
	if (result<0) {
		return result;
	}

	files_increment_count(file_old);
	current_proc[get_cpu()]->files[newfd]=file_old;

	return result;
}

