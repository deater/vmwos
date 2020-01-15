#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

#include "syscalls.h"
#include "vmwos.h"
#include "vlibc.h"

/*****************************************************/
/* String compare functions                          */
/*****************************************************/

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

/*****************************************************/
/* String length functions                           */
/*****************************************************/

int strlen(const char *s) {

	int length=0;

	while(s[length]) length++;

	return length;
}

/*****************************************************/
/* String copy functions                             */
/*****************************************************/


/* At most n bytes of src are copied to dest */
/* If no nul in the first n bytes of src, dest will *not* be nul terminated */
/* If length of src less than n, nuls are padded */
char *strncpy(char *dest, const char *src, uint32_t n) {

	uint32_t i;

	for(i=0; i<n; i++) {
		dest[i]=src[i];
		if (src[i]=='\0') break;
	}
	for(i=i;i<n;i++) {
		dest[i]='\0';
	}

	return dest;

}

/*****************************************************/
/* String search functions                           */
/*****************************************************/

/* Returns a pointer to the first occurrence of the character c
   in the string s, or NULL if not found.
*/

char *strchr (const char *s, int32_t c) {

	do {
		if (*s == c) return (char *)s;
	} while (*s++);

	return NULL;
}


/* Returns a pointer to the last occurrence of
   the character c in the string s
   or NULL if not found.
*/

char *strrchr (const char *s, int32_t c) {

	char *returnval = NULL;

	do {
		if (*s == c) returnval = (char *) s;
	} while (*s++);

	return returnval;
}

/* Search for the substring sub in the string string,
   not including the terminating null characters.
   A pointer to the first occurrence of sub is returned,
   or NULL if the substring is absent.
   If sub points to a string with zero length, the function returns string.
*/

char *strstr (const char *s1, const char *s2) {

	const char *p = s1;
	const int32_t len = strlen(s2);

	/* Find first char, and if that matches then to a strncmp */
	for (; (p = strchr (p, *s2)) != 0; p++) {
		if (strncmp (p, s2, len) == 0) return (char *)p;
	}

	return NULL;
}





int32_t atoi(char *string) {

	int result=0;
	char *ptr;
	int sign=1;

	ptr=string;

	if (*ptr=='-') {
		sign=-1;
		ptr++;
	}

	while((*ptr!='\0') && isdigit(*ptr)) {
		result*=10;
		result+=(*ptr)-'0';
		ptr++;
	}

	return result*sign;
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

#if 0
//void *memcpy_byte(void *dest, const void *src, uint32_t n) {
void *memcpy(void *dest, const void *src, uint32_t n) {

        int i;

        char *d=dest;
        const char *s=src;

        for(i=0;i<n;i++) {
                *d=*s;
                d++;
		s++;
        }

        return dest;
}
#endif

#if 0

void *memcpy_4bytes(void *dest, const void *src, uint32_t n) {

	uint32_t i;

	uint32_t *int_dest,*int_src;
	uint8_t *char_dest,*char_src;

	uint32_t tail;

	tail=n%4;

	int_dest=(uint32_t *)dest;
	int_src=(uint32_t *)src;

	char_dest=(uint8_t *)dest;
	char_src=(uint8_t *)src;


	for(i=0;i<((n-tail)/4);i++) {
		int_dest[i]=int_src[i];
	}

	/* Do trailing edge */
	for(i=0;i<tail;i++) {
		char_dest[i]=char_src[i];
	}

	return dest;
}

#endif


	/* Not optimized */
int32_t memcmp(const void *s1, const void *s2, uint32_t n) {

	int i,result;
	char *c1,*c2;

	c1=(char *)s1;
	c2=(char *)s2;

	for(i=0;i<n;i++) {
		result=c1[i]-c2[i];
		if (result) return result;
	}

	return 0;
}

/* FIXME: not optimized */
void *memmove(void *dest, const void *src, size_t n) {

	int i;
	char *d = dest;
	const char *s = src;

	/* If dest and src same, just return destination */
	if (d==s) {
		return d;
	}

	/* If no overlap, just run memcpy */
	if ((uintptr_t)s-(uintptr_t)d-n <= -2*n) {
		return memcpy(d, s, n);
	}

	/* if desitnation less than src, run forward */
	/* otherwise, copy backwards */
	if (d<s) {
		for(i=0;i<n;i++) {
			*d++ = *s++;
		}
	} else {
		for(i=n-1;i>=0;i--) {
			d[i] = s[i];
		}
	}

	return dest;
}
