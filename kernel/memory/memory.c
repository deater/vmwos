#include <stddef.h>
#include <stdint.h>
#include "boot/hardware_detect.h"

#include "memory/memory.h"
#include "memory/mmu-common.h"

#include "lib/printk.h"
#include "lib/memset.h"
#include "lib/locks.h"
#include "lib/errors.h"

#define MAX_MEMORY	(1024*1024*1024)		// 1GB
#define RESERVED_KERNEL	(16*1024*1024)			// 16MB

#define CHUNK_SIZE	4096

static int memory_debug=0;

static uint32_t memory_mutex=MUTEX_UNLOCKED;

static unsigned int memory_total;

static unsigned int memory_map[MAX_MEMORY/CHUNK_SIZE/32];

static unsigned int max_chunk=0;

/* Mark a chunk as used (1) */
static int memory_mark_used(int chunk) {

	int element,bit;

	element=chunk/32;
	bit=chunk%32;

	memory_map[element]|=1<<bit;

	return 0;
}

/* Mark a chunk as free (0) */
static int memory_mark_free(int chunk) {

	int element,bit;

	element=chunk/32;
	bit=chunk%32;

	memory_map[element]&=~(1<<bit);

	return 0;
}

/* See if a chunk is used */
static int memory_test_used(int chunk) {

	int element,bit;

	element=chunk/32;
	bit=chunk%32;

	return memory_map[element] & (1<<bit);
}

/* Initialize memory */
static int memory_init(unsigned long memory_total,
				unsigned long memory_kernel) {

	int i;

	if (memory_total>MAX_MEMORY) {
		printk("Error!  Too much memory!\n");
		return -ENOMEM;
	}

	if (memory_kernel>RESERVED_KERNEL) {
		printk("Error!  Kernel too big!\n");
		return -ENOMEM;
	}

	printk("Initializing %dMB of memory.  "
		"%dkB reserved (%dKB kernel, %dkB memory map)\n",
		memory_total/1024/1024,
		RESERVED_KERNEL/1024,
		memory_kernel/1024,
		(MAX_MEMORY/CHUNK_SIZE/32)/1024);

	max_chunk=(memory_total/CHUNK_SIZE);

	/* Clear it out, probably not necessary */
	for(i=0;i<max_chunk/32;i++) {
		memory_map[i]=0;
	}

	/* Mark OS area as used */
	for(i=0;i<(memory_kernel/CHUNK_SIZE);i++) {
		memory_mark_used(i);
	}

	return 0;
}

/* Find a free chunk of memory */
/* FIXME: this might have bounds errors */
static int find_free(int num_chunks, int start, int end) {

	int i,j;

	for(i=start;i<end;i++) {
		if (!memory_test_used(i)) {
			for(j=0;j<num_chunks;j++) {
				if (memory_test_used(i+j)) break;
			}
			if ((j==num_chunks) && (i+j<end)) {
				return i;
			}
		}
	}

	return -1;
}

/* Report amount of memory free */
int32_t memory_total_free(void) {

	int32_t total_free=0,i;

	for(i=0;i<max_chunk;i++) {
		if (!memory_test_used(i)) total_free++;
	}

	return total_free*CHUNK_SIZE;

}

/* allocate an area of memory */
/* rounds up to nearest chunk size */
void *memory_allocate(uint32_t size, uint32_t type) {

	int first_chunk;
	int num_chunks;
	int i;
	int start,end;

	if (memory_debug) {
		printk("Allocating memory of size %d bytes\n",size);
	}

	if (size==0) size=1;

	num_chunks = ((size-1)/CHUNK_SIZE)+1;

	if (memory_debug) {
		printk("\tRounding up to %d %d chunks\n",num_chunks,CHUNK_SIZE);
	}

	if (type==MEMORY_KERNEL) {
		start=0;
		end=RESERVED_KERNEL/CHUNK_SIZE;
	}
	else if (type==MEMORY_USER) {
		start=RESERVED_KERNEL/CHUNK_SIZE;
		end=max_chunk;
	}
	else {
		printk("\tUnknown memory allocation type %d\n",type);
		return NULL;
	}

	mutex_lock(&memory_mutex);

	first_chunk=find_free(num_chunks,start,end);

	if (first_chunk<0) {
		printk("Error!  Could not allocate %d of memory!\n",size);
		mutex_unlock(&memory_mutex);
		return NULL;
	}

	for(i=0;i<num_chunks;i++) {
		memory_mark_used(first_chunk+i);
	}

	mutex_unlock(&memory_mutex);

	/* clear memory to zero, both for bss and also security reasons */
	memset((void *)(first_chunk*CHUNK_SIZE),0,num_chunks*CHUNK_SIZE);

	if (memory_debug) {
		printk("MEM: Allocated %d bytes at %x\n",
			size,first_chunk*CHUNK_SIZE);
	}

	return (void *)(first_chunk*CHUNK_SIZE);

}

int32_t memory_free(void *location, uint32_t size) {

	int i;
	int num_chunks;
	int first_chunk;

	if (memory_debug ) {
		int32_t total;

		total=memory_total_free();
		printk("Free before: %d\n",total);
	}

	num_chunks = ((size-1)/CHUNK_SIZE)+1;

	if (memory_debug) {
		printk("Freeing %d bytes (%d CHUNKS) at %x\n",
			size,num_chunks,location);
	}

	first_chunk=(int)location/CHUNK_SIZE;

	/* Poison old memory */
	memset(location,'V',size);

	for(i=0;i<num_chunks;i++) {
		memory_mark_free(first_chunk+i);
	}

	if (memory_debug ) {
		int32_t total;

		total=memory_total_free();
		printk("Free after: %d\n",total);
	}


	return 0;
}

uint32_t memory_get_total(void) {

	return memory_total;
}

void memory_hierarchy_init(unsigned long memory_kernel) {

	uint32_t start,length,i;

	/**************************/
	/* Init Memory Hierarchy  */
	/**************************/

	hardware_get_memory(&start,&length);

	memory_total=length;

	/* Init memory subsystem */
	memory_init(memory_total,memory_kernel);

	/* Setup Memory Hierarchy */
	if ((hardware_get_type()==RPI_MODEL_2B) ||
		(hardware_get_type()==RPI_MODEL_3B) ||
		(hardware_get_type()==RPI_MODEL_3BPLUS)) {

		/* Enable MMU plus caches */
		printk("Enabling MMU with 1:1 Virt/Phys page mapping\n");
		printk("Enabling I/D caches\n");
		setup_pagetable(0,memory_total,RESERVED_KERNEL);
		enable_mmu(1);

		/* Mark the multicore stacks as reserved */
		for(i=0xff4000;i<0x1000000;i+=CHUNK_SIZE) {
			memory_mark_used(i/CHUNK_SIZE);
		}
	}
	else {

		/* Enable L1 i-cache */
		printk("Enabling L1 icache\n");
		enable_l1_icache();

		/* Enable branch predictor */
		printk("Enabling branch predictor\n");
		enable_branch_predictor();

		/* Enable L1 d-cache */
		printk("Enabling MMU with 1:1 Virt/Phys page mapping\n");
		setup_pagetable(0,memory_total,RESERVED_KERNEL);
		enable_mmu(1);
		printk("Enabling L1 dcache\n");
		enable_l1_dcache();
	}

}
