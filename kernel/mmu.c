/* From discussion here:
	http://stackoverflow.com/questions/3439708/how-to-enable-arm1136jfs-arm-v6-mmu-to-have-one-to-one-mapping-between-physica
*/

#include <stdint.h>

/* 1 entry per 1MB, so this covers 4G address space */
#define NUM_PAGE_TABLE_ENTRIES 4096

#define CACHE_DISABLED    0x12
#define CACHE_WRITEBACK   0x1e

void enable_mmu(uint32_t mem_start, uint32_t mem_end) {

	static uint32_t  __attribute__((aligned(16384)))
		page_table[NUM_PAGE_TABLE_ENTRIES];
	int i;
	uint32_t reg;

	/* Set up an identity-mapping for all 4GB */
	/* rw for everyone */
	for (i = 0; i < NUM_PAGE_TABLE_ENTRIES; i++) {
		page_table[i] = i << 20 | (3 << 10) | CACHE_DISABLED;
	}

	/* Then, enable cacheable and bufferable for RAM only */
	for (i = mem_start >> 20; i < mem_end >> 20; i++) {
		page_table[i] = i << 20 | (3 << 10) | CACHE_WRITEBACK;
	}

	/* Copy the page table address to cp15 */
	asm volatile("mcr p15, 0, %0, c2, c0, 0"
		: : "r" (page_table) : "memory");

	/* Set the access control to all-supervisor */
	asm volatile("mcr p15, 0, %0, c3, c0, 0" : : "r" (~0));

	/* Enable the MMU */
	asm("mrc p15, 0, %0, c1, c0, 0" : "=r" (reg) : : "cc");
	reg|=0x1;
	asm volatile("mcr p15, 0, %0, c1, c0, 0" : : "r" (reg) : "cc");
}

void l1_data_cache_enable(void) {
	asm volatile( "  mrc p15, 0, r0, c1, c0, 0" );  // Read c1 into r0
	asm volatile( "  orr r0, r0, #4" );             // Set bit 2: Dcache
	asm volatile( "  mcr p15, 0, r0, c1, c0, 0" );  // Return r0 to c1
}

void l1_data_cache_disable(void) {
	asm volatile( "_disable_data_cache_start_:" );
	asm volatile( "  mrc p15, 0, r15, c7, c14, 3" ); // test, clean and invalidate
	asm volatile( "  bne _disable_data_cache_start_" );

	asm volatile( "  mov r0,#0" );
	asm volatile( "  mcr p15, 0, r0, c7, c5, 0" ); //  invalidate I cache
	asm volatile( "  mcr p15, 0, r0, c7, c10, 4"); //  drain write buffer

	asm volatile( "  mrc p15, 0, r0, c1, c0, 0" );  // Read c1 into r0
	asm volatile( "  bic r0, r0, #4"     );         // Clear bit 2: disable Dcache
	asm volatile( "  mcr p15, 0, r0, c1, c0, 0"  ); // Return r0 to c1
}

void l1_data_cache_clear(void) {
	asm volatile( "_clean_data_cache_start_:" );
	asm volatile( "  mrc p15, 0, r15, c7, c14, 3" ); // test, clean and invalidate
	asm volatile( "  bne _clean_data_cache_start_" );
}

void l1_instruction_cache_enable(void) {
	asm volatile( "  mrc p15, 0, r0, c1, c0, 0" );  // Read c1 into r0
	asm volatile( "  orr r0, r0, #4096" );          // Set bit 12: Icache
	asm volatile( "  mcr p15, 0, r0, c1, c0, 0" );  // Return r0 to c1
}

void l1_instruction_cache_disable(void) {
	asm volatile( "  mrc p15, 0, r0, c1, c0, 0" );  // Read c1 into r0
	asm volatile( "  bic r0, r0, #4096" );          // Clearr bit 12: Icache
	asm volatile( "  mcr p15, 0, r0, c1, c0, 0" );  // Return r0 to c1
}

