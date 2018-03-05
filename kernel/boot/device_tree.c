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

#include <stdint.h>

#include <stddef.h>

/* Not sure why arm cross-compiler does not support this? */
#ifndef	PRIx64
#define PRIx64	"%llx"
#endif

#ifndef STANDALONE_TEST

#include "lib/string.h"
#include "lib/memset.h"
#include "lib/memcpy.h"
#include "lib/errors.h"
#include "lib/printk.h"

#include "boot/device_tree.h"

//#ifndef NULL
//#define NULL (void *)0
//#endif

#else

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#define printk printf
#define strlcpy strncpy

#endif

//static int fdt_verbose=0;

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


int32_t devicetree_setup(uint32_t *dt_ptr) {

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

static void dt_copy_string(uint32_t offset, char *s, uint32_t len) {

	char *string;
	string=(char *)(device_tree.address+device_tree.off_strs+offset);

	strlcpy(s,string,len);
}

static uint32_t dt_parse_prop(uint32_t *tree, int *i,
	char *current_node_name, char *node, char *prop) {

	uint32_t temp,length,j,k;
	char string[DT_STRING_MAXSIZE];
	int printing,saved;
	int start_i;

	start_i=*i;
	string[4]=0;

//	printk("%s:",current_node_name);

	temp=big_to_little(tree[*i]);
	//if (fdt_verbose) printk("\tValue Length: %x\n",temp);
	length=temp;
	(*i)++;

	temp=big_to_little(tree[*i]);
	//if (fdt_verbose) printk("\tstring offset: %x\n",temp);
	dt_copy_string(temp,string,DT_STRING_MAXSIZE);
//	printk("%s",string);


	if ((node==0) || (!strncmp(node,current_node_name,DT_STRING_MAXSIZE))) {

		if (!strncmp(string,prop,DT_STRING_MAXSIZE)) {
			//printk("FOUND!\n");
			return start_i;
		}
	}

	(*i)++;

//	printk("\n\t");
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
//					printk("%c",string[k]);
				}
			}
		}
	}
//	printk("\n\t");

	*i=saved;

	for(j=0;j<(length+3)/4;j++) {

		temp=tree[*i];
		(*i)++;
		memcpy(string,&temp,4);

//		for(k=0;k<4;k++) {
//			printk("%x",string[k]&0xff);
//		}
	}
//	printk("\n");


	return 0;
}

static uint32_t devicetree_prop_int(int start_i,uint32_t *value) {

	uint32_t *tree;

	uint32_t temp,length,i;

	if (!device_tree.valid) return -ENODEV;

	tree=(uint32_t *)(device_tree.address+device_tree.off_struct);

	i=start_i;

	temp=big_to_little(tree[i]);
	//if (fdt_verbose) printk("\tValue Length: %x\n",temp);
	length=temp;
	i++;

	temp=big_to_little(tree[i]);
	//if (fdt_verbose) printk("\tstring offset: %x\n",temp);
	i++;

	temp=big_to_little(tree[i]);
	i++;

	*value=temp;

	if (length!=4) {
		printk("Warning! Asking for int32 but length is %d\n",length);
	}


	return 0;
}


static uint32_t devicetree_prop_string(int start_i,char *string,int len) {


	uint32_t *tree;

	uint32_t temp,length,i,j;
	char *ptr;

	if (!device_tree.valid) return -ENODEV;

	tree=(uint32_t *)(device_tree.address+device_tree.off_struct);

	i=start_i;

	temp=big_to_little(tree[i]);
	//if (fdt_verbose) printk("\tValue Length: %x\n",temp);
	length=temp;
	i++;

	temp=big_to_little(tree[i]);
	//if (fdt_verbose) printk("\tstring offset: %x\n",temp);
	i++;

	ptr=(char *)&tree[i];

	for(j=0;j<length;j++) {
		string[j]=*ptr;
		ptr++;
	}

	return 0;
}

static uint32_t dt_parse_node(uint32_t *tree, int *i,char *node, char *prop) {

	uint32_t temp;
	char *ptr;
	char node_name[DT_STRING_MAXSIZE];
	uint32_t result;

	if (big_to_little(tree[*i])!=FDT_BEGIN_NODE) {
		printk("Expected FDT_BEGIN_NODE, got %x\n",tree[*i]);
		(*i)++;
		return -1;
	}
	(*i)++;

//	printk("Node: ");

	ptr=(char *)&tree[*i];
	strlcpy(node_name,ptr,DT_STRING_MAXSIZE);
//	printk("%s\n",ptr);

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
			result=dt_parse_prop(tree,i,node_name,node,prop);
			if (result>0) return result;
		}
		else if (big_to_little(tree[*i])==FDT_BEGIN_NODE) {
			result=dt_parse_node(tree,i,node,prop);
			if (result>0) return result;
		}
		else if (big_to_little(tree[*i])==FDT_END_NODE) {
			(*i)++;
			break;
		} else {
			printk("UNEXPECTED %x\n",big_to_little(tree[*i]));
			(*i)++;
		}
	}
	return 0;
}


int32_t devicetree_find_reserved(void) {

	int i;
	uint64_t *memory_reserved;

	if (!device_tree.valid) return -ENODEV;

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

	return 0;
}

static int32_t devicetree_find_node_prop(char *node, char *prop) {

	int i;
	uint32_t *tree;
	uint32_t result;

	if (!device_tree.valid) return -ENODEV;

	tree=(uint32_t *)(device_tree.address+device_tree.off_struct);

	i=0;

	while(1) {
		result=dt_parse_node(tree,&i,node,prop);
		if (result>0) return result;

		/* See if at end */
		if (big_to_little(tree[i])==FDT_END) break;
		if (device_tree.off_struct + (i*4) > device_tree.totalsize) {
			break;
		}
	}
	return 0;
}



uint32_t devicetree_find_string(char *node, char *prop,
		char *string, uint32_t len) {

	uint32_t result;

	result=devicetree_find_node_prop(node,prop);
	if (result<0) return result;

	devicetree_prop_string(result,string,len);

	return 0;
}

uint32_t devicetree_find_int(char *node, char *prop, uint32_t *value) {

	uint32_t result;

	result=devicetree_find_node_prop(node,prop);
	if (result<0) {
		return result;
	}

	devicetree_prop_int(result,value);

	return 0;
}


uint64_t devicetree_get_memory(void) {

	uint32_t address_cells,size_cells,result,temp,val_len;
	uint32_t *tree;
	uint64_t address,length,temp64;
	int i;

	devicetree_find_int(NULL,"#address-cells",&address_cells);
	printk("mem: found addr cells: %d\n",address_cells);

	devicetree_find_int(NULL,"#size-cells",&size_cells);
	printk("mem: found size cells: %d\n",size_cells);

	/* Point to memory:reg */
	result=devicetree_find_node_prop("memory","reg");
	if (result<0) {
		return result;
	}

	tree=(uint32_t *)(device_tree.address+device_tree.off_struct);

	i=result;

	temp=big_to_little(tree[i]);
//	printk("\tValue Length: %x\n",temp);
	val_len=temp;
	i++;

	temp=big_to_little(tree[i]);
	//if (fdt_verbose) printk("\tstring offset: %x\n",temp);
	i++;

	/* NOTE: should loop if val_len != 8 */

	if (val_len!=8) {
		printk("ERROR!  Unexpected number of memory entries\n");
	}

	if (address_cells==1) {
		memcpy(&temp,&tree[i],4);
		address=big_to_little(temp);
		i+=1;
	} else if (address_cells==2) {
		memcpy(&temp64,&tree[i],8);
		address=big_to_little64(temp64);
		i+=2;
	} else {
		address=0;
		printk("Error! Unknown addr cell size\n");
	}

	if (size_cells==1) {
		memcpy(&temp,&tree[i],4);
		length=big_to_little(temp);
		i+=1;
	} else if (size_cells==2) {
		memcpy(&temp64,&tree[i],8);
		length=big_to_little64(temp64);
		i+=2;
	} else {
		length=0;
		printk("Error! Unknown size cell size\n");
	}

	printk("Memory found, %llx bytes at address %llx (%dMB)\n",
		length,address,length/1024/1024);

	return length;

}

#ifdef STANDALONE_TEST

#define MAXSIZE 100*1024

int main(int argc, char **argv) {

	char *dt_mem;
	int fd;
	int size;
	char string[DT_STRING_MAXSIZE];
	uint32_t revision=0;

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

	devicetree_setup((uint32_t *)dt_mem);

	devicetree_find_string(NULL,"model",string,DT_STRING_MAXSIZE);

	printf("\t%s\n",string);


	devicetree_find_int("system","linux,revision",&revision);

	printf("\t%x\n",revision);


	devicetree_get_memory();

	return 0;
}


#endif
