#include <stdint.h>
#include "div.h"
#include "printk.h"

/* ARMV6 has no division instruction	*/
/* calculate  q=(dividend/divisor)	*/
/* Not necessarily meant to be speedy 	*/
uint32_t div32(uint32_t dividend, uint32_t divisor) {

	uint32_t q;

	if (divisor==0) {
		printk("Division by zero!\r\n");
		return 0;
	}

	q=0;
	while(1) {
		if (divisor>dividend) break;

		q++;
		divisor+=divisor;
	}

	return q;
}
