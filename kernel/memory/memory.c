#include <stddef.h>
#include <stdint.h>

#include "arch/arm1176/arm1176-mmu.h"

#include "boot/hardware_detect.h"

#include "memory/memory.h"

#include "lib/printk.h"

#define MAX_MEMORY	(1024*1024*1024)
#define CHUNK_SIZE	4096

static unsigned int memory_total;

static unsigned int memory_map[MAX_MEMORY/CHUNK_SIZE/32];

static unsigned int max_chunk=0;

static int memory_mark_used(int chunk) {

	int element,bit;

	element=chunk/32;
	bit=chunk%32;

	memory_map[element]|=1<<bit;

	return 0;
}

static int memory_test_used(int chunk) {

	int element,bit;

	element=chunk/32;
	bit=chunk%32;

	return memory_map[element] & (1<<bit);
}

static int memory_init(unsigned long memory_total,unsigned long memory_kernel) {

	int i;

	if (memory_total>MAX_MEMORY) {
		printk("Error!  Too much memory!\n");
		return -1;
	}

	printk("Initializing %dMB of memory.  "
		"%dkB used by kernel, %dkB used by memory map\n",
		memory_total/1024/1024,
		memory_kernel/1024,
		(MAX_MEMORY/CHUNK_SIZE/32)/1024);

	max_chunk=(memory_total/CHUNK_SIZE);

	/* Clear it out, probably not necessary */
	for(i=0;i<max_chunk/32;i++) {
		memory_map[i]=0;
	}

	/* Mark OS area as used */
	for(i=0;i<memory_kernel/CHUNK_SIZE;i++) {
		memory_mark_used(i);
	}

	return 0;
}

static int find_free(int num_chunks) {

	int i,j;

	for(i=0;i<max_chunk;i++) {
		if (!memory_test_used(i)) {
			for(j=0;j<num_chunks;j++) {
				if (memory_test_used(i+j)) break;
			}
			if (j==num_chunks) {
				return i;
			}
		}
	}

	return -1;
}

int32_t memory_total_free(void) {

	int32_t total_free=0,i;

	for(i=0;i<max_chunk;i++) {
		if (!memory_test_used(i)) total_free++;
	}

	return total_free*CHUNK_SIZE;

}

void *memory_allocate(uint32_t size) {

	int first_chunk;
	int num_chunks;
	int i;

//	printk("Allocating memory of size %d bytes\n",size);

	if (size==0) size=1;

	num_chunks = ((size-1)/CHUNK_SIZE)+1;

	first_chunk=find_free(num_chunks);

	if (first_chunk<0) {
		printk("Error!  Could not allocate %d of memory!\n",size);
		return NULL;
	}

	for(i=0;i<num_chunks;i++) {
		memory_mark_used(first_chunk+i);
	}

	return (void *)(first_chunk*CHUNK_SIZE);

}

int32_t memory_free(void *location, uint32_t size) {

	printk("ERROR: memory_free not implemented yet.\n");

	return 0;
}

uint32_t memory_get_total(void) {

	return memory_total;
}

void memory_hierarchy_init(unsigned long memory_kernel) {

	uint32_t start,length;

	/**************************/
	/* Init Memory Hierarchy  */
	/**************************/

	hardware_get_memory(&start,&length);

	memory_total=length;

	/* Init memory subsystem */
	memory_init(memory_total,memory_kernel);

	if ((hardware_get_type()==RPI_MODEL_2B) ||
		(hardware_get_type()==RPI_MODEL_3B)) {

		/* Enable L1 d-cache */
		printk("Enabling MMU with 1:1 Virt/Phys page mapping\n");
		enable_mmu(0,memory_total,memory_kernel);
	}
	else {

		/* Setup Memory Hierarchy */

		// memset_benchmark(memory_total);

		/* Enable L1 i-cache */
		printk("Enabling L1 icache\n");
		enable_l1_icache();

		/* Enable branch predictor */
		printk("Enabling branch predictor\n");
		enable_branch_predictor();

		/* Enable L1 d-cache */
		printk("Enabling MMU with 1:1 Virt/Phys page mapping\n");
		enable_mmu(0,memory_total,memory_kernel);
		printk("Enabling L1 dcache\n");
		enable_l1_dcache();
	}

}
