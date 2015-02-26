#include <stddef.h>
#include <stdint.h>
#include "printk.h"
#include "syscalls.h"
#include "io.h"

extern int blinking_enabled;

uint32_t __attribute__((interrupt("SWI"))) swi_handler(

	uint32_t r0, uint32_t r1, uint32_t r2, uint32_t r3) {

	register long r7 asm ("r7");
	uint32_t result=0;

	switch(r7) {

		case SYSCALL_WRITE:
			printk("Trying to write: %d %x %d\r\n",
				r0,r1,r2);
			result = write(r0, (char *)r1, (size_t)r2);
			printk("After write, result=%d\r\n",result);
			break;

		case SYSCALL_BLINK:
			if (r0==0) {
				printk("DISABLING BLINK\r\n");
				blinking_enabled=0;
			}
			else {
				printk("ENABLING_BLINK\r\n");
				blinking_enabled=1;
			}
			break;

		default:
			printk("Unknown syscall %d\n",r7);
			break;
	}

	/* The SWI handler code restores all of the registers	*/
	/* Before returning.  To get our result in r0 we have	*/
	/* to push it into the place of the saved r0 on the	*/
	/* stack.						*/

	asm volatile(	"pop {r0}\n"
			"push {%[result]}\n"
		:
                :       [result] "r" (result)
		:);

	return result;

}

