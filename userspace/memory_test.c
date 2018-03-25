#include <stddef.h>
#include <stdint.h>


#define BENCH_SIZE (4*1024*1024)
#define BENCH_ITERATIONS 16

static uint8_t __attribute__((aligned(64))) benchmark[BENCH_SIZE+16];

#define OFFSET 0


#ifdef VMWOS
#include "syscalls.h"
#include "vlibc.h"
#include "vmwos.h"


/* FIXME: implement cycle counters */
/* Or at least, clock_gettime() */
static int64_t get_time_us(void) {

	int64_t value;
	struct timespec t;

	clock_gettime(CLOCK_REALTIME,&t);

	value=(t.tv_sec*1000000ULL)+t.tv_nsec/1000;

	return value;
}

#else

#include <stdio.h>
#include <time.h>


/* FIXME: implement cycle counters */
/* Or at least, clock_gettime() */
static int64_t get_time_us(void) {

	int64_t value;

	struct timespec t;

	clock_gettime(CLOCK_REALTIME,&t);

	value=(t.tv_sec*1000000ULL)+t.tv_nsec/1000;

	return value;
}

#endif






void *memset_byte(void *s, int c, uint32_t n) {

	uint32_t i;
	char *b;

	b=(char *)s;

	for(i=0;i<n;i++) b[i]=c;

	return s;
}


void *memset_4byte(void *s, int c, uint32_t n) {

	uint32_t i;
	uint32_t *b;
	uint8_t *ch;
	uint32_t pattern;
	uint32_t offset;
	uint32_t count;


	pattern=(c&0xff);
	pattern=pattern|(pattern<<8)|(pattern<<16)|(pattern<<24);

	//printk("Writing %x (4 x %x)\n",pattern,c);

	offset=((uint32_t)s)%4;

	ch=(uint8_t *)s;

	/* Do leading edge if unaligned */
	if (offset) {
		for(i=0;i<(4-offset);i++) ch[i]=c;
		b=(uint32_t *)(ch+(4-offset));
		count=n-1;
	}
	else {
		b=(uint32_t *)s;
		count=n;
	}

	/* Do 4-byte chunks */
	for(i=0;i<(count/4);i++) b[i]=pattern;

	/* Do trailing edge if unaligned */
	for(i=0;i<offset;i++) {
//		printk("Tail: setting offset %d\n",(n-offset)+i);
		ch[(n-offset)+i]=c;
	}

	return s;
}


/* based on the version in the Linux kernel */
/* arch/arm/lib/memset.S */
//void *memset_64byte(void *s, int c, uint32_t n) {
void *memset_asm(void *s, int c, uint32_t n) {

	asm("stmfd	sp!, {r4-r8, lr}");	// save registers on stack

	// check if address unaligned
	asm("ands	r3, r0, #3");
	// we return the pointer to the buffer area (memset definition)
	asm("mov	ip, r0");		// ip is r9 = scratch reg

	asm("beq	asm_memset_4bytealigned");

	asm("subs	r2, r2, #4");	// are we copying more than 4?
	asm("blt	asm_memset_lessthan4");	// if not we are done

        asm("cmp	r3, #2");
	asm("strltb	r1, [ip], #1");	// if <2 store 1 byte
	asm("strleb	r1, [ip], #1"); // if <=2 store another byte
	asm("strb	r1, [ip], #1"); // always store at least one byte
	asm("add	r2, r2, r3");   // adjust count (r2 = r2 - (4 - r3))

	asm("asm_memset_4bytealigned:");

	// fill r1 with byte pattern
	asm("orr	r1, r1, r1, lsl #8");	// copy low 8-bits up 8-bits
	asm("orr	r1, r1, r1, lsl #16");	// copy low 16-bits to top

	// going to write 32 bytes at a time
	asm("mov	r3, r1");
	asm("mov	r4, r1");
        asm("mov	r5, r1");
        asm("mov	r6, r1");
        asm("mov	r7, r1");
        asm("mov	r8, r1");
        asm("mov	lr, r1");

	/* Linux checks if count >96 and address > 0xc000 0000	*/
	/* And does something special ?				*/

//	asm("cmp	r2, #96");
//	asm("tstgt	ip, #31");
//	asm("ble	3f");


	asm("asm_memset_loop:");

	// decrement count by 64
	asm("subs	r2, r2, #64");

	// if was greater than 64, write out 64-bytes of value
	asm("stmgeia	ip!, {r1, r3-r8, lr}");
	asm("stmgeia	ip!, {r1, r3-r8, lr}");

	// loop until less than 64
	asm("bgt	asm_memset_loop");


	// if count is 0 we are done
	asm("beq	asm_memset_done");

	// otherwise, need to clean up

	// handle 32-64 case
	asm("tst	r2, #32");
	asm("stmneia	ip!, {r1, r3-r8, lr}");

	/* handle 16-32 case */
	asm("tst	r2, #16");
	asm("stmneia	ip!, {r4-r7}");

	/* handle 8-16 case */
	asm("tst	r2, #8");
	asm("stmneia	ip!, {r1, r3}");

	/* handle 4-8 case */
	asm("tst	r2, #4");
	asm("strne	r1, [ip], #4");

	// We have less than 4 bytes left
	asm("asm_memset_lessthan4:");

	asm("tst	r2, #2");		// if 2 or 3, write 2 out
	asm("strneb	r1, [ip], #1");
	asm("strneb	r1, [ip], #1");
	asm("tst	r2, #1");
	asm("strneb	r1, [ip], #1");		// if 1 or 3, write 1 out

	asm("asm_memset_done:");

	asm("ldmfd	sp!, {r4-r8, lr}");	// restore regs from stack

	return s;
}


/* Test to be sure correct value was written */
static void memset_test(void *addr, int value, int size) {

	int i,errors=0;
	char *b;
	b=(char *)addr;

	printf("\tTesting: ");
	for(i=0;i<size;i++) {
		if (b[i]!=value) {
			printf("Not match at offset %d (%x!=%x)!\n",
				i,b[i],value);
			errors++;
			if (errors>20) break;
		}
	}

	if (b[size+1]==value) {
		errors++;
		printf("Value after the end has wrong value!\n");
	}

	if (errors) printf("Failed!\n");
	else printf("Passed!\n");

}


/* Test setting per-byte memory set */
static void run_memory_benchmark8(int32_t reps) {

	int i;
	uint64_t before,after;
	uint32_t diff;

	before=get_time_us();

	for(i=0;i<reps;i++) {
		memset_byte(benchmark+OFFSET,0xfe,BENCH_SIZE);
	}

	after=get_time_us();

	diff=after-before;

	printf("\tMEMSPEED: %d MB took %dus = %dMB/s\n",
		(BENCH_SIZE*reps)/1024/1024,
		(diff),
		(BENCH_SIZE*reps)/(diff));

	memset_test(benchmark+OFFSET,0xfe,BENCH_SIZE);

}

static void run_memory_benchmark32(int32_t reps) {

	int i;
	uint64_t before,after;
	uint32_t diff;

	before=get_time_us();

	for(i=0;i<reps;i++) {
		memset_4byte(benchmark+OFFSET,0xa5,BENCH_SIZE);
	}

	after=get_time_us();

	diff=after-before;

	printf("\tMEMSPEED: %d MB took %dus = %dMB/s\n",
		(BENCH_SIZE*reps)/1024/1024,
		(diff),
		(BENCH_SIZE*reps)/diff);

	memset_test(benchmark+OFFSET,0xa5,BENCH_SIZE);
}

static void run_memory_benchmark_asm(int32_t reps) {

	int i;
	uint64_t before,after;
	uint32_t diff;

	before=get_time_us();

	for(i=0;i<reps;i++) {
		memset_asm(benchmark+OFFSET,0x78,BENCH_SIZE);
	}

	after=get_time_us();

	diff=after-before;

	printf("\tMEMSPEED: %d MB took %dus =  %dMB/s\n",
		BENCH_SIZE*reps/1024/1024,
		(diff),
		(BENCH_SIZE*reps)/(diff));

	memset_test(benchmark+OFFSET,0x78,BENCH_SIZE);
}

static void memset_benchmark(int32_t reps) {

	/* Naive version */
	printf("8-bit memset\n");
	run_memory_benchmark8(reps);

	/* 32-bit version */
	printf("32-bit memset\n");
	run_memory_benchmark32(reps);

	/* asm version */
	printf("Assembly 64-bit copy\n");
	run_memory_benchmark_asm(reps);

}


int main(int argc, char **argv) {

	int32_t reps;

	if (argc>1) {
		reps=atoi(argv[1]);
	}
	else {
		reps=BENCH_ITERATIONS;
	}

	memset_benchmark(reps);

	return 0;
}
