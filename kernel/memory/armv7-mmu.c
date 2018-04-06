/* This is actually mostly tested on armv8-32 (pi3).  Need to try pi2 */

/* References:
	Bare-metal Boot Code for ARMv8-A Processors

   The page/section references are referring to this:
	ARM Architecture Reference Manual ARMv7-A and ARMv7-R edition

   This pi-forum topic was extremely helpful,
	especially user BR Schnoogle
	https://www.raspberrypi.org/forums/viewtopic.php?f=72&t=205179
*/

#include <stdint.h>

#include "lib/printk.h"
#include "arch/armv7/armv7-mmu.h"

static int mmu_debug=1;

static void tlb_invalidate_all(void) {
	uint32_t reg=0;

	/* TLBIALL */
	asm volatile("mcr p15, 0, %0, c8, c7, 0"
		: : "r" (reg) : "memory");
}

static void icache_invalidate_all(void) {
	uint32_t reg=0;

	/* ICIALLU */
	asm volatile("mcr p15, 0, %0, c7, c5, 1"
		: : "r" (reg) : "memory");
}


/* Code from "Bare-metal Boot Code for ARMv8-A document */
void disable_l1_dcache(void) {
	// Disable L1 Caches.
	asm volatile(
	"mrc	p15, 0, r1, c1, c0, 0	// Read SCTLR.\n"
	"bic	r1, r1, #(0x1 << 2)	// Disable D Cache.\n"
	"mcr	p15, 0, r1, c1, c0, 0	// Write SCTLR.\n"
	);
}

#if 0
/* Code from "Bare-metal Boot Code for ARMv8-A document */
void invalidate_l1_dcache(void) {

	asm volatile(
	"push	{r4, r5, r6, r7, r8, lr}\n"
	"// Invalidate Data cache to create general-purpose code.\n"
	"// Calculate the cache size first and loop through each set + way\n"
	"mov	r0, #0x0		// R0 = 0x0 for L1 dcache 0x2 for L2 dcache\n"
	"mcr	P15, 2, R0, C0, C0, 0	// CSSELR Cache Size Selection Register\n"
	"mrc	P15, 1, R4, C0, C0, 0	// CCSIDR read Cache Size\n"
	"and	R1, R4, #0x7\n"
	"add	R1, R1, #0x4		// r1 = cache line size\n"
	"ldr	R3, =0x7FFF\n"
	"and	R2, R3, R4, LSR #13	// r2 = cache set size\n"
	"ldr	R3, =0x3FF\n"
	"and	R3, R3, R4, LSR #3	// r3 = Cache Associativity Number – 1.\n"
	"clz	R4, R3			// r4 = way position in CISW instruction.\n"
	"mov	R5, #0			// r5 = way loop counter.\n"
"way_loop:\n"
	"mov	R6, #0			// r6 = set loop counter\n"
"set_loop:\n"
	"orr	R7, R0, R5, LSL R4	// Set way.\n"
	"orr	R7, R7, R6, LSL R1	// Set set.\n"
	"mcr	P15, 0, R7, C7, C6, 2	// DCCISW R7.\n"
	"add	R6, R6, #1		// Increment set counter.\n"
	"cmp	R6, R2			// Last set reached yet?\n"
	"ble	set_loop		// If not, iterate set_loop,\n"
	"add	R5, R5, #1		// else, next way.\n"
	"cmp	R5, R3			// Last way reached yet?\n"
	"ble	way_loop		// if not, iterate way_loop.\n"
	"pop	{r4, r5, r6, r7, r8, lr}\n"
	);
}
#endif

/* Code from pi forum thread */

/*
 *************************************************************************
 *
 * invalidate_dcache - invalidate the entire d-cache by set/way
 *
 * Note: for Cortex-A53, there is no cp instruction for invalidating
 * the whole D-cache. Need to invalidate each line.
 *
 *************************************************************************
 */
void invalidate_l1_dcache(void) {

	asm volatile(
	"push	{r0 - r12}\n"
	"mrc	p15, 1, r0, c0, c0, 1		/* read CLIDR */\n"
	"ands	r3, r0, #0x7000000\n"
	"mov	r3, r3, lsr #23			/* cache level value (naturally aligned) */\n"
	"beq	finished\n"
	"mov	r10, #0				/* start with level 0 */\n"
"loop1:\n"
	"add	r2, r10, r10, lsr #1		/* work out 3xcachelevel */\n"
	"mov	r1, r0, lsr r2			/* bottom 3 bits are the Cache type for this level */\n"
	"and	r1, r1, #7			/* get those 3 bits alone */\n"
	"cmp	r1, #2\n"
	"blt	skip				/* no cache or only instruction cache at this level */\n"
	"mcr	p15, 2, r10, c0, c0, 0		/* write the Cache Size selection register */\n"
	"isb					/* isb to sync the change to the CacheSizeID reg */\n"
	"mrc	p15, 1, r1, c0, c0, 0		/* reads current Cache Size ID register */\n"
	"and	r2, r1, #7			/* extract the line length field */\n"
	"add	r2, r2, #4			/* add 4 for the line length offset (log2 16 bytes) */\n"
	"ldr	r4, =0x3ff\n"
	"ands	r4, r4, r1, lsr #3		/* r4 is the max number on the way size (right aligned) */\n"
	"clz	r5, r4				/* r5 is the bit position of the way size increment */\n"
	"ldr	r7, =0x7fff\n"
	"ands	r7, r7, r1, lsr #13		/* r7 is the max number of the index size (right aligned) */\n"
"loop2:\n"
	"mov	r9, r4				/* r9 working copy of the max way size (right aligned) */\n"
"loop3:\n"
	"orr	r11, r10, r9, lsl r5		/* factor in the way number and cache number into r11 */\n"
	"orr	r11, r11, r7, lsl r2		/* factor in the index number */\n"
//	"mcr	p15, 0, r11, c7, c6, 2		/* invalidate by set/way */\n"
	"mcr	p15, 0, r11, c7, c14, 2		/* clean+invalidate by set/way */\n"
	"subs	r9, r9, #1			/* decrement the way number */\n"
	"bge	loop3\n"
	"subs	r7, r7, #1			/* decrement the index */\n"
	"bge	loop2\n"
"skip:\n"
	"add	r10, r10, #2			/* increment the cache number */\n"
	"cmp	r3, r10\n"
	"bgt	loop1\n"
"finished:\n"
	"mov	r10, #0				/* swith back to cache level 0 */\n"
	"mcr	p15, 2, r10, c0, c0, 0		/* select current cache level in cssr */\n"
	"dsb\n"
	"isb\n"
	"pop	{r0-r12}\n"
	);
}


/* By default caches have pseudo-random replacement */
/* This can be configured to round-robin in the control register (bit 14) */



/* We want to cover all 4GB of address space	*/
/* Using 1MB pages, so need 4096 entries	*/
#define NUM_PAGE_TABLE_ENTRIES 4096

/* make sure properly aligned, as the low bits are reserved  */
/* This means we need 14-bit (16k) allignment */

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
          16         8
90c0e = 1001 0000 1100 0000 1110  011=full access
9080e = 1001 0000 1000 0000 1110  010=only root can write
9040e = 1001 0000 0100 0000 1110  001=only root can read/write
90c16 = 1001 0000 1100 0001 0110  011=full access, no cache

The above work.  Other values I tried didn't :(
This is: not-secure, shareable, domain 0, and the rest as described.
*/

#define SECTION_KERNEL	0x9040e		// shared, root-only, cached
#define SECTION_USER	0x90c0e		// shared, any-access, cached
#define SECTION_1GB	0x90416		// root-only, non-cached
//92c10 = 1001 0010 1100 0001 0000
#define SECTION_2GB	0x92c10		// root-only, non-cached
#define SECTION_PERIPH	0x90416		// root-only, non-cached
#define SECTION_DEFAULT	0x90c16		// any-access, non-cached

//#define SECTION_FULL_ACCESS_NO_CACHE	0x10c16


/* Enable a one-to-one physical to virtual mapping using 1MB pagetables */
void enable_mmu(uint32_t mem_start, uint32_t mem_end, uint32_t kernel_end) {

	int i;
	uint32_t reg;

	/* Set up an identity-mapping for all 4GB */
	/* section-short descriptor 1MB pages */


	/* Flush TLB */
	if (mmu_debug) printk("\tInvalidating TLB\n");
	tlb_invalidate_all();
	/* Flush l1-icache */
	if (mmu_debug) printk("\tInvalidating icache\n");
	icache_invalidate_all();
	/* Flush l1-dcache */
	if (mmu_debug) printk("\tInvalidating dcache\n");
	disable_l1_dcache();
	/* Need to flush l2-cache too? */

	/* As a baseline, Set 1:1 mapping for all memory */
	/* Cache disabled, supervisor access only */

	if (mmu_debug) {
		printk("\tSetting 1:1, cache disabled "
			"for %d page table entries\n",
			NUM_PAGE_TABLE_ENTRIES);
	}

	/* First set some default values for all */
	for (i = 0; i < NUM_PAGE_TABLE_ENTRIES; i++) {
		page_table[i] = i << 20 | SECTION_DEFAULT;

	}

	/* Enable supervisor only and cachable for kernel */
	for (i = (mem_start >> 20); i < (kernel_end >> 20); i++) {
		page_table[i] = i << 20 | SECTION_KERNEL;
	}
	if (mmu_debug) {
		printk("\tSetting cachable+kernel only for %x to %x, "
			"actual %x to %x\n",
			mem_start,kernel_end,
			mem_start&0xfff00000,
			kernel_end&0xfff00000);
	}

	/* Enable cachable and readable by all for rest of RAM */
	for (i = kernel_end >> 20; i < mem_end >> 20; i++) {
		page_table[i] = i << 20 | SECTION_USER;
	}
	if (mmu_debug) {
		printk("\tSetting cachable+any for %x to %x, "
			"actual %x to %x\n",
			kernel_end,mem_end,
			kernel_end&0xfff00000,mem_end&0xfff00000);
	}

	/* Set from 1GB-2GB */
	for (i = 0x40000000 >> 20; i < 0x80000000 >> 20; i++) {
		page_table[i] = i << 20 | SECTION_1GB;
	}

	/* Set from 2GB-4GB */
	for (i = 0x40000000 >> 20; i < 0x80000000 >> 20; i++) {
		page_table[i] = i << 20 | SECTION_2GB;
	}




	/* TTBCR : Translation Table Base Control Register */
	/* B3.5.4 (1330) */
	/* Choice of using TTBR0 (user) vs TTBR1 (kernel) */
	/* This is based on address range, also TTBCR.N */
	/* N is bottom 3 bits, if 000 then TTBR1 not used */
	/* We set N to 0, meaning only use TTBR0 */
	asm volatile("mrc p15, 0, %0, c2, c0, 2" : "=r" (reg) : : "cc");
	if (mmu_debug) printk("\tTTBCR before = %x\n",reg);
	reg=0;
	asm volatile("mcr p15, 0, %0, c2, c0, 2" : : "r" (reg) : "cc");

	/* See B.4.1.43 */
	/* DACR: Domain Access Control Register */
	/* All domains, set manager access (no faults for accesses) */
	if (mmu_debug) printk("\tInitialize DACR\n");
	reg=0x55555555;	// all domains, client access
	asm volatile("mcr p15, 0, %0, c3, c0, 0" : : "r" (reg): "cc");

	/* Initialize SCTLR.AFE */
	/* This boots with value 0, but set to 0 anyway */
	if (mmu_debug) printk("\tInitialize SCTLR.AFE\n");
	asm volatile("mrc p15, 0, %0, c1, c0, 0" : "=r" (reg) : : "cc");
	if (mmu_debug) printk("\tSCTLR before AFE = %x\n",reg);
	reg&=~SCTLR_ACCESS_FLAG_ENABLE;
	asm volatile("mcr p15, 0, %0, c1, c0, 0" : : "r" (reg) : "cc");

	/* TTBR0 (VMSA): Translation Table Base Register 0 */
	/* See B.4.1.154 (page 1729) */
	/* This is the userspace pagetable, can be per-process */

	/* Bits 31-N are the address of the table */
	/* Low bits are various config options, we leave them at 0 */
	/* FIXME: might need to do something if SMP support added */
	if (mmu_debug) {
		printk("\tSetting page table to %x\n",page_table);
		printk("\tPTE[0] = %x\n",page_table[0]);
	}

	reg=(uint32_t)page_table;
	reg|=0x6a;		// 0110 1010
				// IRGN = 10 : inner write-through cache
				// NOS = 1 : inner sharable
				// RGN = 01 : normal mem, outer writeback/allocate
				// S = 1 : sharable
	asm volatile("mcr p15, 0, %0, c2, c0, 0"
		: : "r" (reg) : "memory");

#if 0
	/* SMP is implemented in the CPUECTLR register on armv8? */
	uint32_t reg2;

	if (mmu_debug) printk("Enabling SMPEN\n");
	asm volatile("mrrc p15, 1, %0, %1, c15" :  "=r" (reg), "=r"(reg2):: "cc");
	reg|=(1<<6);	// Set SMPEN.
	asm volatile("mcrr p15, 1, %0, %1, c15" : : "r" (reg), "r"(reg2):"cc");
#endif


	/* See B.4.1.130 on page 1707 */
	/* SCTLR, VMSA: System Control Register */
	/* Enable the MMU by setting the M bit (bit 1) */
	asm volatile("mrc p15, 0, %0, c1, c0, 0" : "=r" (reg) : : "cc");
	if (mmu_debug) printk("\tSCTLR before = %x\n",reg);
	reg|=SCTLR_MMU_ENABLE;

/* Enable caches!  Doesn't quite work */
#if 1
	reg|=SCTLR_CACHE_ENABLE;
	reg|=SCTLR_ICACHE_ENABLE;
#endif
	asm volatile("mcr p15, 0, %0, c1, c0, 0" : : "r" (reg) : "cc");

	asm volatile("dsb");	/* barrier */
	asm volatile("isb");	/* barrier */

	asm volatile("mrc p15, 0, %0, c1, c0, 0" : "=r" (reg) : : "cc");
	if (mmu_debug) printk("\tSCTLR after = %x\n",reg);
}

void enable_l1_dcache(void) {

	/* still issues with this on pi3 */

	/* load control register to r0 */
	asm volatile( "mrc p15, 0, r0, c1, c0, 0" );
	/* set bit 12: enable dcache */
	asm volatile( "orr r0, r0, #4" );
	/* store back out to control register */
	asm volatile( "mcr p15, 0, r0, c1, c0, 0" );
}


void enable_l1_icache(void) {

	/* still issues with this on pi3 */

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

	/* Still not sure how to do this on Pi3 */

#if 0
	/* load control register to r0 */
	asm volatile( "mrc p15, 0, r0, c1, c0, 0" );
	/* clear bit 12: enable icache */
	asm volatile( "bic r0, r0, #4096" );
	/* store back out to control register */
	asm volatile( "mcr p15, 0, r0, c1, c0, 0" );
#endif
}

/* Z-bit */
/* Also see 3.2.8 Auxiliary Control Register */
/* Bit 2 (SB) = static branch prediction (on by default) */
/* Bit 1 (DB) = dynamic branch prediction (on by default) */
/* Bit 0 (RS) = return stack prediction (on by default) */
void enable_branch_predictor(void) {

	/* On Pi3 this comes up already enabled? */

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

	/* Not, it's probably not possible to disable on ARMv8 */

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



/* If we-reuse memory for executable code we need to flush out of icache */
/* as when we store out to memory it goes in dcache but icache doesn't see */
/* the update and we end up running old code */
void flush_icache(void) {

	uint32_t reg=0;

//; Enter this code with <Rx> containing the new 32-bit instruction.
// STR <Rx>, [instruction location]
// DCCMVAU [instruction location] ; Clean data cache by MVA to point of unification
// DSB
//; Ensure visibility of the data cleaned from the cache
// ICIMVAU [instruction location] ; Invalidate instruction cache by MVA to PoU
// BPIMVAU [instruction location] ; Invalidate branch predictor by MVA to PoU
//DSB ; Ensure completion of the invalidations
// ISB ; Synchronize fetched instruction stream


//	ICIALLU c c7 0 c5 0 32-bit WO Instruction cache invalidate all
//	BPIALL c c7 0 c5 6 32-bit WO Branch predictor invalidate all -

	asm volatile(	"mcr p15, 0, %0, c7, c5, 0\n"	// ICIALLU
			"mcr p15, 0, %0, c7, c5, 6\n"	// BPIALL
			"dsb\n"
			"isb\n"
			: : "r" (reg): "cc");

}

void flush_dcache(uint32_t start_addr, uint32_t end_addr) {

	uint32_t mva;

	// 1559
	// DCCIMVAC, Data Cache Clean and Invalidate by MVA to PoC, VMSA
	// 1643, 1496
	// DCCIMVAC c c7 0 c14 1
	// DCCIMVAU c c7 0 c14 1

	// clean and invalidate data or unified cache line
	// DCCIMVAC c c7 0 c14 1

//	invalidate_l1_dcache();
#if 1
	for(mva=(start_addr&0xfffffff0);mva<=(end_addr&0xfffffff0);mva+=16) {

		asm volatile("mcr p15, 0, %0, c7, c14, 1\n"
			: : "r" (mva) : "memory");
	}
#endif
	asm volatile("dsb\n"  // DSB
		: : : "memory");
	asm volatile("isb\n"  // ISB
		: : : "memory");

}
