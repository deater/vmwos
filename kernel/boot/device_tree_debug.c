/* This code decodes the flattened device tree provided by the bootloader */

/* See Section 5 of the "Devicetree Specification Release v0.2" */
/* From devicetree.org */

/* Original references */
/* "Device Trees Everywhere" by Gibson and Herrenschmidt */

#define FDT_BEGIN_NODE	1
#define FDT_END_NODE	2
#define FDT_PROP	3
#define FDT_NOP		4
#define FDT_END		9

#include <inttypes.h>
#include <stdint.h>

/* Not sure why arm cross-compiler does not support this? */
#ifndef	PRIx64
#define PRIx64	"%llx"
#endif

#ifndef STANDALONE_TEST

#include "lib/string.h"
#include "lib/memset.h"
#include "lib/memcpy.h"

#include "errors.h"

#include "lib/printk.h"

#include "boot/device_tree.h"

#else

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#define printk printf

#endif

static int fdt_verbose=0;

struct device_tree_type {
	void *address;
	uint32_t valid;
	uint32_t magic;
	uint32_t totalsize;
	uint32_t off_struct;
	uint32_t off_strs;
	uint32_t off_rsvmap;
	uint32_t version;
	uint32_t last_comp_ver;
	uint32_t boot_cpu_id;
	uint32_t size_strs;
} device_tree;

/* We assume the pi is always little endian */
static uint32_t big_to_little(uint32_t big) {

	uint32_t little;

	little=(	((big&0xff000000)>>24) |
			((big&0x00ff0000)>>8) |
			((big&0x0000ff00)<<8) |
			((big&0x000000ff)<<24)	);

	return little;
}

/* We assume the pi is always little endian */
static uint64_t big_to_little64(uint64_t big) {

	uint64_t little;

	little=(	((big&0xff00000000000000ULL)>>48) |
			((big&0x00ff000000000000ULL)>>32) |
			((big&0x0000ff0000000000ULL)>>16) |
			((big&0x000000ff00000000ULL)>>8) |
			((big&0x00000000ff000000ULL)<<8) |
			((big&0x0000000000ff0000ULL)<<16) |
			((big&0x000000000000ff00ULL)<<32) |
			((big&0x00000000000000ffULL)<<48));


	return little;
}


int32_t devicetree_decode(uint32_t *dt_ptr) {

	uint32_t *dt;

	dt=dt_ptr;

	device_tree.valid=0;

	device_tree.magic=big_to_little(dt[0]);

	if (device_tree.magic!=0xd00dfeed) {
		return -ENODEV;
	}

	device_tree.valid=1;

	device_tree.address=(void *)dt_ptr;

	device_tree.totalsize=big_to_little(dt[1]);
	device_tree.off_struct=big_to_little(dt[2]);
	device_tree.off_strs=big_to_little(dt[3]);
	device_tree.off_rsvmap=big_to_little(dt[4]);
	device_tree.version=big_to_little(dt[5]);
	device_tree.last_comp_ver=big_to_little(dt[6]);
	if (device_tree.version>=2) {
		device_tree.boot_cpu_id=big_to_little(dt[7]);
	}

	if (device_tree.version>=3) {
		device_tree.size_strs=big_to_little(dt[8]);
	}

	return 0;

}

static void dt_print_string(uint32_t offset) {

	char *string;
	string=(char *)(device_tree.address+device_tree.off_strs+offset);

	printk("%s",string);
}

static uint32_t dt_parse_prop(uint32_t *tree, int *i) {

	uint32_t temp,length,j,k;
	char string[5];
	int printing,saved;

	string[4]=0;

	printk("\t");

	temp=big_to_little(tree[*i]);
	if (fdt_verbose) printk("\tValue Length: %x\n",temp);
	length=temp;
	(*i)++;

	temp=big_to_little(tree[*i]);
	if (fdt_verbose) printk("\tstring offset: %x\n",temp);
	dt_print_string(temp);
	(*i)++;

	printk("\t");
	printing=1;

	saved=*i;

	for(j=0;j<(length+3)/4;j++) {

		temp=tree[*i];
		(*i)++;
		memcpy(string,&temp,4);

		if (printing) {
			for(k=0;k<4;k++) {
				if (string[k]==0) {
					printing=0;
					break;
				}
				else {
					printk("%c",string[k]);
				}
			}
		}
	}
	printk("\n\t");

	*i=saved;

	for(j=0;j<(length+3)/4;j++) {

		temp=tree[*i];
		(*i)++;
		memcpy(string,&temp,4);

		for(k=0;k<4;k++) {
			printk("%x",string[k]&0xff);
		}
	}
	printk("\n");


	return 0;
}

static uint32_t dt_parse_node(uint32_t *tree, int *i) {

	uint32_t temp;
	char *ptr;

	if (big_to_little(tree[*i])!=FDT_BEGIN_NODE) {
		printk("Expected FDT_BEGIN_NODE, got %x\n",tree[*i]);
		(*i)++;
		return -1;
	}
	(*i)++;

	printk("Node: ");

	ptr=(char *)&tree[*i];
	printk("%s\n",ptr);

	/* Skip to end of padding */
	while(1) {
		temp=big_to_little(tree[*i]);
		(*i)++;
//		printk("%x ",temp);
		if ((temp&0xff)==0) break;
	}


	while(1) {

		if (big_to_little(tree[*i])==FDT_NOP) {
			(*i)++;
			continue;
		}
		if (big_to_little(tree[*i])==FDT_PROP) {
			(*i)++;
			dt_parse_prop(tree,i);
		}
		else if (big_to_little(tree[*i])==FDT_BEGIN_NODE) {
			dt_parse_node(tree,i);
		}
		else if (big_to_little(tree[*i])==FDT_END_NODE) {
			if (fdt_verbose) printk("FDT_END_NODE\n");
			(*i)++;
			break;
		} else {
			printk("UNEXPECTED %x\n",big_to_little(tree[*i]));
			(*i)++;
		}
	}
	return 0;
}


void devicetree_dump(void) {

	int i;
	uint64_t *memory_reserved;
	uint32_t *tree;

	if (!device_tree.valid) return;

	printk("Device tree:\n");
	printk("\tMagic: 0x%x\n",device_tree.magic);
	printk("\tTotalsize:  %d\n",device_tree.totalsize);
	printk("\tOff Struct: 0x%x\n",device_tree.off_struct);
	printk("\tOff strs:   0x%x\n",device_tree.off_strs);
	printk("\tOff rsvmap: 0x%x\n",device_tree.off_rsvmap);
	printk("\tVersion:    %d\n",device_tree.version);
	printk("\tLastCompVer:%d\n",device_tree.last_comp_ver);
	if (device_tree.version>=2) {
		printk("\tBootCpuID: %d\n",device_tree.boot_cpu_id);
	}
	if (device_tree.version>=3) {
		printk("\tSizeStrs: %d\n",device_tree.size_strs);
	}

	/* Print reserved memory table */
	memory_reserved=(uint64_t *)
		(device_tree.address+device_tree.off_rsvmap);

	printk("Reserved memory:\n");

	i=0;
	while(1) {
		if ((memory_reserved[i]==0) &&
			(memory_reserved[i+1]==0)) break;

		printk("\t0x%" PRIx64 " bytes starting at "
			"address 0x%" PRIx64 "\n",
			big_to_little64(memory_reserved[i+1]),
			big_to_little64(memory_reserved[i]));
		i+=2;
	}

	tree=(uint32_t *)(device_tree.address+device_tree.off_struct);
	printk("Device Tree:\n");
	i=0;

	while(1) {
		dt_parse_node(tree,&i);

		/* See if at end */
		if (big_to_little(tree[i])==FDT_END) break;
		if (device_tree.off_struct + (i*4) > device_tree.totalsize) {
			break;
		}
	}
}


int32_t devicetree_raw_dump(uint32_t *dt_ptr) {

	uint32_t *dt;
	uint32_t magic,size;
	char *ptr;

	int i,count;

	dt=dt_ptr;

	magic=big_to_little(dt[0]);

	if (magic!=0xd00dfeed) {
		return -ENODEV;
	}

	size=big_to_little(dt[1]);

	printk("BEGIN DT DUMP OF SIZE %d\n",size);

	count=0;
	ptr=(char *)dt_ptr;
	for(i=0;i<size;i++) {
		if ((count&0xf)==0) printk("\n");
		printk("%d ",ptr[i]);
		count++;
	}

	printk("\nEND DT DUMP\n");

	return 0;

}

#ifdef STANDALONE_TEST

#define MAXSIZE 100*1024

int main(int argc, char **argv) {

	char *dt_mem;
	int fd;
	int size;

	dt_mem=calloc(MAXSIZE,sizeof(char));
	if (dt_mem==NULL) {
		fprintf(stderr,"Error opening!\n");
		return -1;
	}

	fd=open("pi3.dt",O_RDONLY);
	if (fd<0) {
		fprintf(stderr,"ERROR opening pi3.dt\n");
		return -1;
	}

	size=read(fd,dt_mem,MAXSIZE);

	printf("Read %d bytes\n",size);

	close(fd);

	devicetree_decode((uint32_t *)dt_mem);
	devicetree_dump();

	return 0;
}


#endif
