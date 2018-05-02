#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

#ifdef TEST
#include <stdio.h>
#else
#include "syscalls.h"
#include "vlibc.h"
#endif

/* ARMV6 has no division instruction	*/
/* calculate  q=(dividend/divisor)	*/
/* Not necessarily meant to be speedy 	*/
uint32_t __aeabi_uidiv(uint32_t dividend, uint32_t divisor) {

	uint32_t q;
	uint32_t new_d;

	if (divisor==0) {
		printf("Division by zero!\n");
		return 0;
	}

	q=0;
	new_d=divisor;
	while(1) {
		if (new_d>dividend) break;

		q++;
		new_d+=divisor;
	}

	return q;
}

/* ARMV6 has no division instruction	*/
/* calculate  q=(dividend/divisor)	*/
/* Not necessarily meant to be speedy 	*/
int32_t __aeabi_idiv(int32_t dividend, int32_t divisor) {

	uint32_t q;
	uint32_t new_d;
	int32_t sign=1;

	if (divisor==0) {
		printf("Division by zero!\n");
		return 0;
	}

	if ((dividend<0) && (divisor>=0)) {
		sign=-1;
	}

	if ((dividend>=0) && (divisor<0)) {
		sign=-1;
	}

	q=0;
	new_d=divisor;
	while(1) {
		if (new_d>dividend) break;

		q++;
		new_d+=divisor;
	}

	return q*sign;
}


/* Not necessarily meant to be speedy				*/
/* This is complicated because we return q in r0 and r in r1?	*/
int32_t __aeabi_idivmod(int32_t dividend, int32_t divisor) {

	uint32_t q,r;
	uint32_t new_d;
	int32_t sign=1;

	if (divisor==0) {
		printf("Division by zero!\n");
		return 0;
	}

	if ((dividend<0) && (divisor>=0)) {
		sign=-1;
	}

	if ((dividend>=0) && (divisor<0)) {
		sign=-1;
	}

	q=0;
	new_d=divisor;
	while(1) {
		if (new_d>dividend) break;

		q++;
		new_d+=divisor;
	}

	r=dividend-(q*divisor);



//	printf("q=%d r=%d\n",q,r);

	/* Return remainder in r1 */
	asm volatile("mov	r1, %0\n"
                : : "r" (r) : "memory");

	return q*sign;
}

#ifdef TEST

int main(int argc, char **argv) {

	int i;

	for(i=0;i<1024;i++) {
		__aeabi_idivmod(i,13);
	}

	return 0;
}
#endif


