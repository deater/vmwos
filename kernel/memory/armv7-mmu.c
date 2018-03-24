#include <stdint.h>

#include "lib/printk.h"
#include "arch/armv7/armv7-mmu.h"

static int debug=1;

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

/* We want a 1MB coarse page table descriptor */
/* B.3.5.1, p1326 */
/* All mappings global and executable */
/* 31-20 = section base address
   19 = NS (not secure)
   18 = 0 (section 0 or supersection 1)
   17 = NG (not global)
   16 = S (sharable)
   15 = AP[2]
   14-12 = TEX[2:0] = Type extension field
   11-10 - AP[1:0] (access permission bits)
   9 = Implementation defined
   8-5 = domain
   4 = XN
   3,2 = C,B determinte caching behavior, see Table b3-10
   1,0 = 1,0 - for coarse section 1MB pages
*/

/* TEX=0 */
/* Domain=1, C=0,B=0, noncachable (Table B3-10) */
#define CACHE_DISABLED		0x22		// 0010 0010
/* Domain=1, C=1,B=1, writeback cache, no alloc on write (Table B3-10) */
#define CACHE_WRITEBACK		0x2e		// 0010 1110

/* Table B3-6 */
#define AP_RW_KERNEL		((0<<15)|(0<<11))
#define AP_RW_ANY		((0<<15)|(1<<11))
#define AP_RO_KERNEL		((1<<15)|(0<<11))
#define AP_RW_ANY		((1<<15)|(1<<11))

/* Enable a one-to-one physical to virtual mapping using 1MB pagetables */
/* This uses the ARMv5 compatible interface, not native ARMv6 */
/* Mark RAM has writeback, but disable cache for non-RAM */
void enable_mmu(uint32_t mem_start, uint32_t mem_end, uint32_t kernel_end) {

	int i;
	uint32_t reg;

	/* Set up an identity-mapping for all 4GB */
	/* section-short descriptor 1MB pages */
	/* See table B3-6 for list of AP bit settings */
	/* On bootup SCTLR.AFE is set to 0 meaning simplified values */

	/* AP[2:1] (bits 15 and 11) */
	/* 00 read/write kernel */
	/* 01 read/write any */
	/* 10 read-only kernel */
	/* 11 read-only any */

	/* As a baseline, Set 1:1 mapping for all memory */
	/* Cache disabled, supervisor access only */

	printk("\tSetting 1:1, cache disabled for %d page table entries\n",
			NUM_PAGE_TABLE_ENTRIES);

	for (i = 0; i < NUM_PAGE_TABLE_ENTRIES; i++) {
		page_table[i] = i << 20 | (AP_RW_KERNEL)
					| CACHE_DISABLED;
	}

	printk("\tSetting cachable+kernel only for addresses %x to %x\n",
			mem_start,kernel_end);

	/* Enanble supervisor only and cachable for kernel */
	for (i = (mem_start >> 20); i < (kernel_end >> 20); i++) {
		page_table[i] = i << 20 | (AP_RW_KERNEL)
					| CACHE_WRITEBACK;
	}

	printk("\tSetting cachable+any access for addresses %x to %x\n",
			kernel_end,mem_end);

	/* Enable cachable and readable by all for rest */
	for (i = kernel_end >> 20; i < mem_end >> 20; i++) {
		page_table[i] = i << 20 | (AP_RW_ANY) | CACHE_WRITEBACK;
	}

//1404
	/* TTBCR : Translation Table Base Control Register */
	/* B3.5.4 (1330) */
	/* Choice of using TTBR0 (user) vs TTBR1 (kernel) */
	/* This is based on address range, also TTBCR.N */
	/* N is bottom 3 bits, if 000 then TTBR1 not used */
	// MRC p15, 0, <Rt>, c2, c0, 2
	// MCR p15, 0, <Rt>, c2, c0, 2

	/* TTBR0 (VMSA): Translation Table Base Register 0 */
	/* See B.4.1.154 (page 1729) */
	/* This is the userspace pagetable, can be per-process */

	/* Bits 31-N are the address of the table */
	/* Low bits are various config options, we leave them at 0 */
	/* FIXME: might need to do something if SMP support added */
	printk("\tSetting page table to %x\n",page_table);
	printk("\tPTE[0] = %x\n",page_table[0]);

	asm volatile("mcr p15, 0, %0, c2, c0, 0"
		: : "r" (page_table) : "memory");


	/* See B.4.1.43 */
	/* DACR: Domain Access Control Register */
	/* All domains, set manager access (no faults for accesses) */
	asm volatile("mcr p15, 0, %0, c3, c0, 0" : : "r" (~0));

	/* See B.4.1.130 on page 1707 */
	/* SCTLR, VMSA: System Control Register */
	/* Enable the MMU by setting the M bit (bit 1) */
	asm("mrc p15, 0, %0, c1, c0, 0" : "=r" (reg) : : "cc");
	printk("\tSCTLR before = %x\n",reg);
	reg|=SCTLR_MMU_ENABLE;
	asm volatile("mcr p15, 0, %0, c1, c0, 0" : : "r" (reg) : "cc");

}


/* Also note that you *must* enable the MMU before using the dcache	*/
/* Why?  hard to find in ARM manuals.  They do have a FAQ for ARM9	*/
/* Why must I enable the MMU to use the D-Cache but not for the I-Cache?*/
/* TLDR: by default the dcache would cache everything, including MMIO	*/
/*       accesses, so you need the MMU enabled so you can mark the MMIO */
/*       regions as non-cachable					*/

void enable_l1_dcache(void) {
	/* TODO */
#if 0
	/* load control register to r0 */
	asm volatile( "mrc p15, 0, r0, c1, c0, 0" );
	/* set bit 12: enable dcache */
	asm volatile( "orr r0, r0, #4" );
	/* store back out to control register */
	asm volatile( "mcr p15, 0, r0, c1, c0, 0" );
#endif
}

/* See 3.2.22 */
void disable_l1_dcache(void) {
	/* FIXME */
	/* Need to clear out and invalidate all entries in cache first */
	/* Also may need to disable L1 icache and branch-target buffer too */
}

/* See 1176 manual, 3.2.7 */
void enable_l1_icache(void) {
	/* TODO */
#if 0
	/* load control register to r0 */
	asm volatile( "mrc p15, 0, r0, c1, c0, 0" );
	/* set bit 12: enable icache */
	asm volatile( "orr r0, r0, #4096" );
	/* store back out to control register */
	asm volatile( "mcr p15, 0, r0, c1, c0, 0" );
#endif
}

void disable_l1_icache(void) {
	/* TODO */
#if 0
	/* load control register to r0 */
	asm volatile( "mrc p15, 0, r0, c1, c0, 0" );
	/* clear bit 12: enable icache */
	asm volatile( "bic r0, r0, #4096" );
	/* store back out to control register */
	asm volatile( "mcr p15, 0, r0, c1, c0, 0" );
#endif
}

/* See 1176 manual, 3.2.7 */
/* Z-bit */
/* Also see 3.2.8 Auxiliary Control Register */
/* Bit 2 (SB) = static branch prediction (on by default) */
/* Bit 1 (DB) = dynamic branch prediction (on by default) */
/* Bit 0 (RS) = return stack prediction (on by default) */
void enable_branch_predictor(void) {
	/* TODO */

#if 0
	/* load control register to r0 */
	asm volatile( "mrc p15, 0, r0, c1, c0, 0" );
	/* set bit 11: enable branch predictor */
	asm volatile( "orr r0, r0, #2048" );
	/* store back out to control register */
	asm volatile( "mcr p15, 0, r0, c1, c0, 0" );
#endif
}

void disable_branch_predictor(void) {

	/* TODO */

#if 0
	/* load control register to r0 */
	asm volatile( "mrc p15, 0, r0, c1, c0, 0" );
	/* clear bit 12: enable icache */
	asm volatile( "bic r0, r0, #2048" );
	/* store back out to control register */
	asm volatile( "mcr p15, 0, r0, c1, c0, 0" );
#endif
}


#if 0
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
#endif

/* see 3.2.3 in the 1176 document */
/* cp15/c0/1 */
void l1_cache_detect(void) {

	/* TODO */

#if 0
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
#endif

}


