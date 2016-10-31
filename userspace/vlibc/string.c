#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

#include "syscalls.h"
#include "vmwos.h"


int strncmp(const char *s1, const char *s2, uint32_t n) {

	int i=0,r;

	while(1) {

		if (i==n) return 0;

		r=s1[i]-s2[i];
		if (r!=0) return r;

		i++;
	}

	return 0;
}

int strlen(const char *s) {

	int length=0;

	while(s[length]) length++;

	return length;
}

int32_t atoi(char *string) {

	int result=0;
	char *ptr;

	ptr=string;

	while(*ptr!=0) {
		result*=10;
		result+=(*ptr)-'0';
		ptr++;
	}

	return result;
}

void *memset(void *s, int c, uint32_t n) {

	// save registers on stack
	asm("stmfd      sp!, {r4-r8, lr}");

	// check if address unaligned
	asm("ands       r3, r0, #3");

	// we return the pointer to the buffer area (memset definition)
	asm("mov        ip, r0");	// ip is r9 = scratch reg

	asm("beq        asm_memset_4bytealigned");

	asm("subs       r2, r2, #4");   // are we copying more than 4?
	asm("blt        asm_memset_lessthan4"); // if not we are done

	asm("cmp        r3, #2");
	asm("strltb     r1, [ip], #1"); // if <2 store 1 byte
	asm("strleb     r1, [ip], #1"); // if <=2 store another byte
	asm("strb       r1, [ip], #1"); // always store at least one byte
	asm("add        r2, r2, r3");   // adjust count (r2 = r2 - (4 - r3))

	asm("asm_memset_4bytealigned:");
	// fill r1 with byte pattern
	asm("orr        r1, r1, r1, lsl #8");   // copy low 8-bits up 8-bits
	asm("orr        r1, r1, r1, lsl #16");  // copy low 16-bits to top

	// going to write 32 bytes at a time
	asm("mov        r3, r1");
	asm("mov        r4, r1");
	asm("mov        r5, r1");
	asm("mov        r6, r1");
	asm("mov        r7, r1");
	asm("mov        r8, r1");
	asm("mov        lr, r1");

	/* Linux checks if count >96 and address > 0xc000 0000  */
	/* And does something special ?                         */

//	asm("cmp        r2, #96");
//	asm("tstgt      ip, #31");
//	asm("ble        3f");


	asm("asm_memset_loop:");
	// decrement count by 64
	asm("subs       r2, r2, #64");

	// if was greater than 64, write out 64-bytes of value
	asm("stmgeia    ip!, {r1, r3-r8, lr}");
	asm("stmgeia    ip!, {r1, r3-r8, lr}");

	// loop until less than 64
	asm("bgt        asm_memset_loop");


	// if count is 0 we are done
	asm("beq        asm_memset_done");

	// otherwise, need to clean up

	// handle 32-64 case
	asm("tst        r2, #32");
	asm("stmneia    ip!, {r1, r3-r8, lr}");

	/* handle 16-32 case */
	asm("tst        r2, #16");
	asm("stmneia    ip!, {r4-r7}");
	/* handle 8-16 case */
	asm("tst        r2, #8");
	asm("stmneia    ip!, {r1, r3}");

	/* handle 4-8 case */
	asm("tst        r2, #4");
	asm("strne      r1, [ip], #4");

	// We have less than 4 bytes left
	asm("asm_memset_lessthan4:");

	asm("tst        r2, #2");               // if 2 or 3, write 2 out
	asm("strneb     r1, [ip], #1");
	asm("strneb     r1, [ip], #1");
	asm("tst        r2, #1");
	asm("strneb     r1, [ip], #1");         // if 1 or 3, write 1 out

	asm("asm_memset_done:");

	asm("ldmfd      sp!, {r4-r8, lr}");     // restore regs from stack

	return s;
}

