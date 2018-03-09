#include <stdint.h>
#include "lib/div.h"
#include "lib/printk.h"

/* ARMV6 has no division instruction	*/
/* calculate  q=(dividend/divisor)	*/
/* Not necessarily meant to be speedy 	*/
uint32_t __aeabi_uidiv(uint32_t dividend, uint32_t divisor) {

	uint32_t q;
	uint32_t new_d;

	if (divisor==0) {
		printk("Division by zero!\n");
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

#if 0

int main(int argc, char **argv) {

	int i;

	for(i=0;i<100;i++) {
		printf("%d/10=%d\n",
			i,div32(i,10));
	}
	printf("88 %d\n",div32(16*1024,88*1000/64));
	printf("33 %d\n",div32(16*1024,33*1000/64));
	printf("1 %d\n",div32(16*1024,1*1000/64));

	return 0;

}

#endif
