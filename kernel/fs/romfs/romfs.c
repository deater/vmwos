#include <stddef.h>
#include <stdint.h>

#include "memory.h"
#include "lib/printk.h"
#include "lib/string.h"
#include "lib/endian.h"

/* Initrd hack */
#include "../../../userspace/initrd.h"

struct romfs_header_t {
	char magic[8];
	int size;
	int checksum;
	char volume_name[17];
	int first_offset;
};

struct romfs_file_header_t {
	int addr;
	int next;
	int type;
	int special;
	int size;
	int checksum;
	int filename_start;
	int data_start;
};


int open_romfs_file(char *name,
		struct romfs_file_header_t *file);


int load_romfs(char *name,char **binary_start,char **stack_start,
		int *size, int *stack_size) {

	struct romfs_file_header_t file;
	int result;

	result=open_romfs_file(name,&file);
	if (result<0) {
		return result;
	}

	*size=file.size;
	*stack_size=8192;

	/* Allocate Memory */
	*binary_start=(char *)memory_allocate(*size);
	*stack_start=(char *)memory_allocate(*stack_size);

	/* Load executable */
	memcpy(*binary_start,initrd_image+file.data_start,*size);

	return 0;
}


int romfs_read(void *buffer, int *offset, int size) {

	memcpy(buffer,initrd_image+*offset,size);
	(*offset)+=size;

	return 0;
}

int open_romfs_file(char *name,
		struct romfs_file_header_t *file) {

	int debug=0;
	int temp_int;
	unsigned char ch;
	struct romfs_header_t header;
	int offset=0;

	char buffer[16];

	/* Read header */
	romfs_read(header.magic,&offset,8);
	if (memcmp(header.magic,"-rom1fs-",8)) {
		printk("Wrong magic number!\n");
		return -1;
	}

	if (debug) printk("Found romfs filesystem!\n");

	/* Read size */
	romfs_read(&temp_int,&offset,4);
	header.size=ntohl(temp_int);

	if (debug) printk("Size: %d bytes\n",header.size);

	/* Read checksum */
	romfs_read(&temp_int,&offset,4);
	header.checksum=ntohl(temp_int);

	if (debug) printk("Checksum: %x\n",header.size);
	/* FIXME: validate checksum */

	/* Read volume name */
	/* FIXME, various overflow possibilities */
	/* We only record last 16 bytes in header */
	/* We really don't care about volume name */
	while(1) {
		romfs_read(buffer,&offset,16);
		memcpy(header.volume_name,buffer,16);
		if (buffer[15]==0) break;	/* NUL terminated */
	}
	if (debug) printk("Volume: %s\n",header.volume_name);

	while(1) {
		file->addr=offset;

		/* Next */
		romfs_read(&temp_int,&offset,4);
		file->next=ntohl(temp_int)&~0xf;
		file->type=ntohl(temp_int)&0xf;

		/* Special */
		romfs_read(&temp_int,&offset,4);
		file->special=ntohl(temp_int);
		/* Size */
		romfs_read(&temp_int,&offset,4);
		file->size=ntohl(temp_int);
		/* Checksum */
		romfs_read(&temp_int,&offset,4);
		file->checksum=ntohl(temp_int);

		file->filename_start=offset;
		while(1) {
			romfs_read(buffer,&offset,16);
			if (buffer[15]==0) break;	/* NUL terminated */
		}

		file->data_start=offset;

		offset=file->filename_start;

		if (!strncmp(name,
			(char *)initrd_image+file->filename_start,
			strlen(name))) return 0;

		if (debug) {
			while(1) {
				romfs_read(&ch,&offset,1);
				//printk("Read %d at %d\n",ch,offset);
				if (ch==0) break;
				printk("%c",ch);
			}

			printk("\n");
			printk("\tAddr: 0x%x\n",file->addr);
			printk("\tNext: 0x%x\n",file->next);
			printk("\tType: 0x%x\n",file->type);
			printk("\tSize: %d\n",file->size);
			printk("\tChecksum: %x\n",file->checksum);
		}

		offset=file->next;

		if (file->next==0) break;
	}

	return -1;
}
