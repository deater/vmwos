/* From discussion here:
 http://stackoverflow.com/questions/3439708/how-to-enable-arm1136jfs-arm-v6-mmu-to-have-one-to-one-mapping-between-physica
*/

#include <stdint.h>

#include "lib/printk.h"
#include "arch/arm1176/arm1176-mmu.h"

/* From the ARM1176JZF-S Technical Reference Manual	*/
/* The processor has separate L1I and L1D caches	*/
/* Separate I and D tightly-coupled memory (TCM)	*/
/* Two micro-TLBs backed by a main TLB			*/

/* Caches are VIPT */
/* Linesize is 32 bytes (8 words) */
/* On BCM2835 cache sizes are 16kb and 4-way */

/* By default caches have pseudo-random replacement */
/* This can be configured to round-robin in the control register (bit 14) */



/* We want to cover all 4GB of address space	*/
/* Using 1MB pages, so need 4096 entries	*/
#define NUM_PAGE_TABLE_ENTRIES 4096

/* make sure properly aligned, as the low bits are reserved  */
/* we're assuming here that 14 bits are reserved? */
uint32_t  __attribute__((aligned(16384))) page_table[NUM_PAGE_TABLE_ENTRIES];

/* We want a 1MB coarse ARMv5 page table descriptor */
/* see 6.11.1 and 6.12.2 and 6.6.1 */
/* All mappings global and executable */
/* 31-20 = section base address
   19 = NS (not shared)
   18 = 0
   17-15 = SBZ (should be zero)
   14-12 = TEX = Type extension field
   11-10 - AP (access permission bits)
   9 = P - has to do with alizaing
   8-5 = domain
   4 = 0
   3,2 = C,B determinte caching behavior, see Tables 6.2 and 6.3
   1,0 = 1,0 - for ARMv5 1MB pages
*/

/* Domain=1, C=0,B=0, noncachable (Table 6.3) */
#define CACHE_DISABLED		0x12
/* Domain=1, C=1,B=1, writeback cache, no alloc on write (Table 6.3) */
#define CACHE_WRITEBACK		0x1e

/* Table 3-151 */
#define AP_NO_ACCESS		0x0
#define AP_SUPERVISOR_ONLY	0x1
#define AP_USER_READ_ONLY	0x2
#define AP_FULL_ACCESS		0x3


/* Enable a one-to-one physical to virtual mapping using 1MB pagetables */
/* This uses the ARMv5 compatible interface, not native ARMv6 */
/* Mark RAM has writeback, but disable cache for non-RAM */
void setup_pagetable(uint32_t mem_start, uint32_t mem_end, uint32_t kernel_end) {

	int i;

	/* Set up an identity-mapping for all 4GB, ARMv5 1MB pages */
	/* See figure 6-12 */
	/* See table 3-151 for list of AP bit settings */


	/* AP (bits 11 and 10) = 11 = R/W for everyone */

	/* As a baseline, Set 1:1 mapping for all memory */
	/* Cache disabled, supervisor access only */
	for (i = 0; i < NUM_PAGE_TABLE_ENTRIES; i++) {
		page_table[i] = i << 20 | (AP_SUPERVISOR_ONLY << 10)
					| CACHE_DISABLED;
	}

	/* Enable supervisor only and cachable for kernel */
	for (i = (mem_start >> 20); i < (kernel_end >> 20); i++) {
		page_table[i] = i << 20 | (AP_SUPERVISOR_ONLY << 10)
					| CACHE_WRITEBACK;
	}

	/* Enanble cachable and readable by all for rest */
	for (i = kernel_end >> 20; i < mem_end >> 20; i++) {
		page_table[i] = i << 20 | (3 << 10) | CACHE_WRITEBACK;
	}

}

/* Enable a one-to-one physical to virtual mapping using 1MB pagetables */
/* This uses the ARMv5 compatible interface, not native ARMv6 */
/* Mark RAM has writeback, but disable cache for non-RAM */
void enable_mmu(int debug) {

	uint32_t reg;

	/* Copy the page table address to cp15 */
	/* Translation Table, Base 0 */
	/* See 3.2.13 */
	/* Bits 31-N are the address of the table */
	/* Low bits are various config options, we leave them at 0 */
	asm volatile("mcr p15, 0, %0, c2, c0, 0"
		: : "r" (page_table) : "memory");

	/* See 3.2.16 */
	/* Set the access control register */
	/* All domains, set client access (no faults for accesses) */
	reg=0x55555555;
	asm volatile("mcr p15, 0, %0, c3, c0, 0" : : "r" (reg));

	/* See 3.2.7 */
	/* Set the Control Register */
	/* Enable the MMU by setting the M bit */
	asm("mrc p15, 0, %0, c1, c0, 0" : "=r" (reg) : : "cc");
	reg|=0x1;
	asm volatile("mcr p15, 0, %0, c1, c0, 0" : : "r" (reg) : "cc");
}

/* See 1176 manual, 3.2.7 */

/* Also note that you *must* enable the MMU before using the dcache	*/
/* Why?  hard to find in ARM manuals.  They do have a FAQ for ARM9	*/
/* Why must I enable the MMU to use the D-Cache but not for the I-Cache?*/
/* TLDR: by default the dcache would cache everything, including MMIO	*/
/*       accesses, so you need the MMU enabled so you can mark the MMIO */
/*       regions as non-cachable					*/

void enable_l1_dcache(void) {
	/* load control register to r0 */
	asm volatile( "mrc p15, 0, r0, c1, c0, 0" );
	/* set bit 12: enable dcache */
	asm volatile( "orr r0, r0, #4" );
	/* store back out to control register */
	asm volatile( "mcr p15, 0, r0, c1, c0, 0" );
}

/* See 3.2.22 */
void disable_l1_dcache(void) {
	/* FIXME */
	/* Need to clear out and invalidate all entries in cache first */
	/* Also may need to disable L1 icache and branch-target buffer too */
}

/* See 1176 manual, 3.2.7 */
void enable_l1_icache(void) {
	/* load control register to r0 */
	asm volatile( "mrc p15, 0, r0, c1, c0, 0" );
	/* set bit 12: enable icache */
	asm volatile( "orr r0, r0, #4096" );
	/* store back out to control register */
	asm volatile( "mcr p15, 0, r0, c1, c0, 0" );
}

void disable_l1_icache(void) {
	/* load control register to r0 */
	asm volatile( "mrc p15, 0, r0, c1, c0, 0" );
	/* clear bit 12: enable icache */
	asm volatile( "bic r0, r0, #4096" );
	/* store back out to control register */
	asm volatile( "mcr p15, 0, r0, c1, c0, 0" );
}

/* See 1176 manual, 3.2.7 */
/* Z-bit */
/* Also see 3.2.8 Auxiliary Control Register */
/* Bit 2 (SB) = static branch prediction (on by default) */
/* Bit 1 (DB) = dynamic branch prediction (on by default) */
/* Bit 0 (RS) = return stack prediction (on by default) */
void enable_branch_predictor(void) {
	/* load control register to r0 */
	asm volatile( "mrc p15, 0, r0, c1, c0, 0" );
	/* set bit 11: enable branch predictor */
	asm volatile( "orr r0, r0, #2048" );
	/* store back out to control register */
	asm volatile( "mcr p15, 0, r0, c1, c0, 0" );
}

void disable_branch_predictor(void) {
	/* load control register to r0 */
	asm volatile( "mrc p15, 0, r0, c1, c0, 0" );
	/* clear bit 12: enable icache */
	asm volatile( "bic r0, r0, #2048" );
	/* store back out to control register */
	asm volatile( "mcr p15, 0, r0, c1, c0, 0" );
}



static uint32_t convert_size(uint32_t value) {

	switch(value) {
		case 0:	return 0; /* 0.5kB, not supported */
		case 1:	return 1; /* not supported */
		case 2:	return 2; /* not supported */
		case 3: return 4;
		case 4: return 8;
		case 5:	return 16;
		case 6: return 32;
		case 7:	return 64;
		case 8: return 128; /* not supported */
		default: return 0;
	}
}

/* see 3.2.3 in the 1176 document */
/* cp15/c0/1 */
void l1_cache_detect(void) {

	uint32_t reg;
	uint32_t size,assoc,blocksize,res;

	asm("mrc p15, 0, %0, c0, c0, 1" : "=r" (reg) : : "cc");

	/* 28-25 = type, 1110 on 1176 */
	/* 24 = separate I and D caches */

	res=!!(reg&(1<<24));
	size=convert_size((reg>>18)&0xf);
	assoc=(reg>>15)&0x7;
	blocksize=(reg>>12)&3;

	printk("Detected L1 data cache: "
		"%d size, %d assoc, %d blocksize, %d restrict\n",
		size,assoc,blocksize,res);

	res=!!(reg&(1<<11));
	size=convert_size((reg>>6)&0xf);
	assoc=(reg>>3)&0x7;
	blocksize=(reg>>0)&3;

	printk("Detected L1 instruction cache: "
		"%d size, %d assoc, %d blocksize, %d restrict\n",
		size,assoc,blocksize,res);
}


/* If we-reuse memory for executable code we need to flush out of icache */
/* as when we store out to memory it goes in dcache but icache doesn't see */
/* the update and we end up running old code */
void flush_icache(void) {

	/* On ARM1176 this uses the cache operations register */

	// DSB
	//	MCR p15, 0, <Rd>, c7, c10, 4
	// Flush prefetch buffer
	//	MCR p15, 0, <Rd>, c7, c5, 4
	// Invalidate both caches plus branch target:
	//	MCR p15, 0, <Rd>, c7, c7, 0
	// Invalidate Instruction Cache and branch target cache
	//	MCR p15, 0, <Rd>, c7, c5, 0

//	asm volatile("mcr p15, 0, %0, c7, c5, 0\n" // invalidate icache+btc
//			"mcr p15,0,%0,c7,c10,4\n"  // DSB
//		: : "r" (0) : "memory");


	// ARM ERRATA 411920 says icache flush can fail
	// this is the workaround Linux uses

	asm volatile(	"mov	r0, #0\n"
			"mrs	r1, cpsr	@ save status register\n"
			"cpsid	ifa		@ disable interrupts\n"
			"mcr	p15, 0, r0, c7, c5, 0	@ invalidate I-cache\n"
			"mcr	p15, 0, r0, c7, c5, 0	@ invalidate I-cache\n"
			"mcr	p15, 0, r0, c7, c5, 0	@ invalidate I-cache\n"
			"mcr	p15, 0, r0, c7, c5, 0	@ invalidate I-cache\n"
			"msr	cpsr_cx, r1	@ restore interrupts\n"
			"nop\n"
			"nop\n"
			"nop\n"
			"nop\n"
			"nop\n"
			"nop\n"
			"nop\n"
			"nop\n"
			"nop\n"
			"nop\n"
			"nop\n		@ ARM Ltd recommends at least\n"
			"nop\n		@ 11 NOPs\n"
		: : : "r0","r1");


}

void flush_dcache(uint32_t start_addr, uint32_t end_addr) {

	/* On ARM1176 this uses the cache operations register */

	// DSB
	//	MCR p15, 0, <Rd>, c7, c10, 4
	// DB
	//	MCR p15,0,<Rd>,c7,c10,5
	// Invalidate Data Cache Line, using MVA
	// MVA might be a fcse (fast-context switch extension) thing
	// 	MCR p15, 0, <Rd>, c7, c6, 1
	// Invalidate Data Cache Range
	//	MCRR p15,0,<End Address>,<Start Address>,c6
	// Clear and Invalidate Data Cache Range
	//	MCRR p15,0,<End Address>,<Start Address>,c14



	// invalidate dcahce range
	asm volatile("mcrr p15, 0, %0, %1, c14\n"
			: : "r" (end_addr), "r" (start_addr) : "memory");

//	asm volatile("mcr p15,0,%0,c7,c10,5\n"  // DMB
//		: : "r" (0) : "memory");

	asm volatile("mcr p15,0,%0,c7,c10,4\n"  // DSB
		: : "r" (0) : "memory");
}
