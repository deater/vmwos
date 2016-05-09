#include <stddef.h>
#include <stdint.h>
#include "lib/printk.h"
#include "syscalls.h"
#include "drivers/console/console_io.h"
#include "drivers/framebuffer/framebuffer.h"
#include "drivers/framebuffer/framebuffer_console.h"
#include "fs/files.h"
#include "time.h"
#include "interrupts.h"
#include "bcm2835_periph.h"
#include "mmio.h"
#include "drivers/thermal/thermal.h"
#include "drivers/random/bcm2835_rng.h"
#include "process.h"
#include "vfork.h"
#include "exec.h"
#include "exit.h"
#include "wait.h"

extern int blinking_enabled;

/* Note!  Do not call a SWI from supervisor mode */
/* as the svc_lr and svc_spr can get corrupted   */

uint32_t swi_handler_c(
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


        printk("SWI PC=%x SPSR=%x SP=%x\n",entry_pc,entry_spsr,entry_sp);

#endif

	uint32_t result=0;

//	printk("Starting syscall %d\n",r7);

	switch(r7) {
		case SYSCALL_EXIT:
			printk("Process exiting with %d\n",r0);
			exit(r0);
			break;

		case SYSCALL_READ:
//			printk("Trying to read: %d %x %d\n",r0,r1,r2);
			result=read(r0,(char *)r1,(size_t)r2);
			break;

		case SYSCALL_WRITE:
//			printk("Trying to write: %d %x %d\n",
//				r0,r1,r2);
			result=write(r0,(char *)r1,(size_t)r2);
			break;

		case SYSCALL_OPEN:
			result=open((char *)r0,r1,r2);
			break;

		case SYSCALL_CLOSE:
			result=close(r0);
			break;

		case SYSCALL_WAITPID:
			printk("Trying to waitpid on pid %d\n",r0);
			waitpid(r0,(int32_t *)r1,r2);
			break;

		case SYSCALL_EXECVE:
			printk("Trying to exec %s\n",(char *)r0);
			execve((char *)r0,(char *)r1,(char *)r2);
			break;

		case SYSCALL_STAT:
			result=stat((char *)r0,(struct stat *)r1);
			break;

		case SYSCALL_TIME:
			result=time_since_boot();
			break;

		case SYSCALL_GETPID:
			result=process[current_process].pid;
			break;

		case SYSCALL_IOCTL:
			printk("UNIMPLEMENTED SYSCALL: IOCTL %x %x %x\n",
				r0,r1,r2);
			result=-1;
			break;

		case SYSCALL_GETDENTS:
			result=getdents(r0,(struct vmwos_dirent *)r1,r2);
			break;

		case SYSCALL_NANOSLEEP:
			printk("UNIMPLEMENTED SYSCALL: NANOSLEEP\n");
			result=-1;
			break;

		case SYSCALL_VFORK:
			printk("Trying to vfork\n");
			result=vfork();
			printk("Returning %d to parent\n",result);
			sched_yield();
			printk("vfork: should never get here\n");
			break;

		case SYSCALL_BLINK:
			if (r0==0) {
				printk("DISABLING BLINK\n");
				blinking_enabled=0;
			}
			else {
				printk("ENABLING_BLINK\n");
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
				process[which].status=PROCESS_STATUS_READY;
			}
			}
			break;

		case SYSCALL_STOP:
			{
			int which;

			which=r0-'0';

			if ((which>0) && (which<10)) {
				process[which].status=PROCESS_STATUS_SLEEPING;
			}
			}
			break;

		case SYSCALL_TB1:
			result=framebuffer_tb1();
			break;

		case SYSCALL_REBOOT:
			/* See https://www.raspberrypi.org/forums/viewtopic.php?f=72&t=53862 */
			mmio_write(PM_WDOG, PM_PASSWORD | 1);	/* timeout = 1/16th of a second? */
			mmio_write(PM_RSTC, PM_PASSWORD | PM_RSTC_WRCFG_FULL_RESET);
			result = -1;
			break;


		case SYSCALL_TEMPERATURE:
			result=thermal_read();
			break;

		case SYSCALL_RANDOM:
			result=bcm2835_rng_read((uint32_t *)r0);
			break;

		default:
			printk("Unknown syscall %d\n",r7);
			break;
	}

	return result;

}

