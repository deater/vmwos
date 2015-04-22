#include <stddef.h>
#include <stdint.h>
#include "printk.h"
#include "syscalls.h"
#include "io.h"
#include "framebuffer.h"
#include "framebuffer_console.h"


extern int blinking_enabled;

/* Note!  Do not call a SWI from supervisor mode */
/* as the svc_lr and svc_spr can get corrupted   */

uint32_t __attribute__((interrupt("SWI"))) swi_handler(

	uint32_t r0, uint32_t r1, uint32_t r2, uint32_t r3) {

	register long r7 asm ("r7");
	uint32_t result=0;

	switch(r7) {
		case SYSCALL_READ:
//			printk("Trying to read: %d %x %d\r\n",r0,r1,r2);
			if (r0==0) {
				result=console_read((char *)r1,(size_t)r2);
			}
			else {
				printk("Attempting to read from unsupported fd %d\n",r0);
				result=-1;
			}
			break;

		case SYSCALL_WRITE:
//			printk("Trying to write: %d %x %d\r\n",
//				r0,r1,r2);
			if ((r0==1) || (r0==2)) {
				result = console_write((char *)r1, (size_t)r2);
//				printk("After write, result=%d\r\n",result);
			}
			else {
				printk("Attempting to write unsupported fd %d\n",r0);
				result=-1;
			}
			break;

		case SYSCALL_IOCTL:
			printk("UNIMPLEMENTED SYSCALL: IOCTL\r\n");
			result=-1;
			break;

		case SYSCALL_NANOSLEEP:
			printk("UNIMPLEMENTED SYSCALL: NANOSLEEP\r\n");
			result=-1;
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

		case SYSCALL_SETFONT:
			framebuffer_console_setfont(r0-'0');
			break;

		case SYSCALL_GRADIENT:
			result=framebuffer_gradient();
			break;

		case SYSCALL_TB1:
			result=framebuffer_tb1();

		default:
			printk("Unknown syscall %d\n",r7);
			break;
	}

//	printk("Returning from syscall: %x\r\n",result);

#if 0
	register long *sp __asm__("sp");

	printk("sp=%x\n",sp);
	printk("lr=%x\n",sp[-0]);
	printk("ip=%x\n",sp[-1]);
	printk("r7=%x\n",sp[-2]);
	printk("r6=%x\n",sp[-3]);
	printk("r5=%x\n",sp[-4]);
	printk("r4=%x\n",sp[-5]);
	printk("r3=%x\n",sp[-6]);
	printk("r2=%x\n",sp[-7]);
	printk("r1=%x\n",sp[-8]);
	printk("r0=%x\n",sp[-9]);
#endif

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

