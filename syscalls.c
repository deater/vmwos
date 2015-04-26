#include <stddef.h>
#include <stdint.h>
#include "printk.h"
#include "syscalls.h"
#include "io.h"
#include "framebuffer.h"
#include "framebuffer_console.h"
#include "scheduler.h"
#include "time.h"
#include "interrupts.h"


extern int blinking_enabled;

/* Note!  Do not call a SWI from supervisor mode */
/* as the svc_lr and svc_spr can get corrupted   */

uint32_t __attribute__((interrupt("SWI"))) swi_handler(

	uint32_t r0, uint32_t r1, uint32_t r2, uint32_t r3) {

	register long r7 asm ("r7");

	/* Keep running interrupts while inside of SWI */
//	enable_interrupts();

#if 0

	long entry_pc,entry_spsr,entry_sp;

	asm volatile(
		"mov      %[entry_pc], lr\n"
		: [entry_pc]"=r"(entry_pc)
		:
		:
		);

	asm volatile(
		"MRS      %[entry_spsr], spsr\n"
		: [entry_spsr]"=r"(entry_spsr)
		:
		:
		);

	asm volatile(
                "mov      %[entry_sp], sp\n"
                : [entry_sp]"=r"(entry_sp)
                :
                :
                );


        printk("SWI PC=%x SPSR=%x SP=%x\r\n",entry_pc,entry_spsr,entry_sp);

#endif

	uint32_t result=0;

//	printk("Starting syscall %d\r\n",r7);

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

		case SYSCALL_TIME:
			result=time_since_boot();
			break;

		case SYSCALL_GETPID:
			result=process[current_process].pid;
			break;

		case SYSCALL_IOCTL:
			printk("UNIMPLEMENTED SYSCALL: IOCTL %x %x %x\r\n",
				r0,r1,r2);
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

		case SYSCALL_RUN:
			{
			int which;

			which=r0-'0';

			if ((which>0) && (which<10)) {
				process[which].ready=1;
			}
			}
			break;

		case SYSCALL_TB1:
			result=framebuffer_tb1();
			break;

		default:
			printk("Unknown syscall %d\n",r7);
			break;
	}


	/* The SWI handler code restores all of the registers	*/
	/* Before returning.  To get our result in r0 we have	*/
	/* to push it into the place of the saved r0 on the	*/
	/* stack.						*/
#if 0
	asm volatile(
		"MRS      %[entry_spsr], spsr\n"
		: [entry_spsr]"=r"(entry_spsr)
		:
		:
		);

	printk("SPSR exit=%x\r\n",entry_spsr);
#endif

	/* FIXME: This is a hack and fragile */
	/* Need to get result onto user stack */

	asm volatile("str %[result],[sp,#0]\n"
		:	/* output */
		:       [result] "r" (result) /* input */
		:);	/* clobber */

	return result;

}

