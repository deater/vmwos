#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#include "lib/printk.h"

#include "time.h"
#include "arch/arm1176/arm1176-mmu.h"
#include "drivers/pmu/arm-pmu.h"
#include "lib/div.h"

#include "lib/memset.h"

#include "lib/memory_benchmark.h"

#define MEMORY_BENCHMARK 1

#if (MEMORY_BENCHMARK==1)

static void memset_test(void *addr, int value, int size) {

	int i,errors=0;
	char *b;
	b=(char *)addr;

	printk("\tTesting: ");
	for(i=0;i<size;i++) {
		if (b[i]!=value) {
			printk("Not match at offset %d (%x!=%x)!\n",
				i,b[i],value);
			errors++;
			if (errors>20) break;
		}
	}

	if (b[size+1]==value) {
		errors++;
		printk("Value after the end has wrong value!\n");
	}

	if (errors) printk("Failed!\n");
	else printk("Passed!\n");

}

#define BENCH_SIZE (1024*1024)
#define BENCH_ITERATIONS 16
uint8_t __attribute__((aligned(64))) benchmark[BENCH_SIZE+16];

#define OFFSET 0

static void run_memory_benchmark(void) {

	int i;
	uint32_t before,after;

	before=read_cycle_counter();

	for(i=0;i<BENCH_ITERATIONS;i++) {
		memset_byte(benchmark+OFFSET,0xfe,BENCH_SIZE);
	}

	after=read_cycle_counter();

	printk("\tMEMSPEED: %d MB took %d cycles %dMB/s\n",
		BENCH_SIZE*BENCH_ITERATIONS,
		(after-before),
		div32(16*700000,((after-before)/1000)));

	memset_test(benchmark+OFFSET,0xfe,BENCH_SIZE);

}

static void run_memory_benchmark32(void) {

	int i;
	uint32_t before,after;

	before=read_cycle_counter();

	for(i=0;i<BENCH_ITERATIONS;i++) {
		memset_4byte(benchmark+OFFSET,0xa5,BENCH_SIZE);
	}

	after=read_cycle_counter();

	printk("\tMEMSPEED: %d MB took %d cycles %dMB/s\n",
		BENCH_SIZE*BENCH_ITERATIONS,
		(after-before),
		div32(16*700000,((after-before)/1000)));

	memset_test(benchmark+OFFSET,0xa5,BENCH_SIZE);
}

static void run_memory_benchmark_asm(void) {

	int i;
	uint32_t before,after;

	before=read_cycle_counter();

	for(i=0;i<BENCH_ITERATIONS;i++) {
		memset(benchmark+OFFSET,0x78,BENCH_SIZE);
	}

	after=read_cycle_counter();

	printk("\tMEMSPEED: %d MB took %d cycles %dMB/s\n",
		BENCH_SIZE*BENCH_ITERATIONS,
		(after-before),
		div32(16*700000,((after-before)/1000)));

	memset_test(benchmark+OFFSET,0x78,BENCH_SIZE);
}

void memset_benchmark(uint32_t memory_total, uint32_t kernel_end) {

	/* Run some memory benchmarks */
	printk("\nRunning Memory benchmarks %x %x\n",
		benchmark+OFFSET,(uint32_t)memset);
	printk("Default memory:\n");
	run_memory_benchmark();

	/* Enable L1 i-cache */
	enable_l1_icache();
	printk("L1 icache enabled:\n");
	run_memory_benchmark();

	/* Enable branch predictor */
	enable_branch_predictor();
	printk("Branch predictor enabled:\n");
	run_memory_benchmark();

	/* Enable L1 d-cache */
	enable_mmu(0,memory_total,kernel_end);
	enable_l1_dcache();
	printk("L1 dcache enabled:\n");
	run_memory_benchmark();

	/* 32-bit version */
	printk("32-bit copy\n");
	run_memory_benchmark32();

	/* asm version */
	printk("Assembly 64-byt copy\n");
	run_memory_benchmark_asm();

}

#endif
