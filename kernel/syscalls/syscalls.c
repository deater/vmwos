#include <stddef.h>
#include <stdint.h>

#include "lib/dmesg.h"
#include "lib/printk.h"
#include "lib/errors.h"
#include "lib/smp.h"

#include "syscalls/syscalls.h"

#include "drivers/console/console_io.h"
#include "drivers/framebuffer/framebuffer.h"
#include "drivers/framebuffer/framebuffer_console.h"
#include "drivers/bcm2835/bcm2835_io.h"
#include "drivers/bcm2835/bcm2835_periph.h"
#include "drivers/thermal/thermal.h"
#include "drivers/random/bcm2835_rng.h"
#include "drivers/audio/audio.h"

#include "fs/files.h"
#include "fs/inodes.h"
#include "fs/superblock.h"

#include "time/time.h"

#include "interrupts/interrupts.h"
#include "interrupts/ipi.h"

#include "processes/process.h"
#include "processes/scheduler.h"
#include "processes/waitpid.h"
#include "processes/exit.h"

#include "syscalls/vfork.h"
#include "syscalls/exec.h"
#include "syscalls/getcpu.h"
#include "syscalls/uname.h"
#include "syscalls/sysinfo.h"
#include "syscalls/times.h"
#include "syscalls/nanosleep.h"

#include "memory/memory.h"

extern int blinking_enabled;

/* Note!  Do not call a SWI from supervisor mode */
/* as the svc_lr and svc_spr can get corrupted   */

uint32_t swi_handler_c(
	uint32_t r0, uint32_t r1, uint32_t r2, uint32_t r3) {

	register long r4 asm ("r4");
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

	uint32_t result=-ENOSYS;
	uint64_t result64;

//	printk("Starting syscall %d\n",r7);

	switch(r7) {
		case SYSCALL_EXIT:	/* 1 */
			//printk("Process exiting with %d\n",r0);
			exit(r0);
			break;

		case SYSCALL_READ:	/* 3 */
//			printk("Trying to read: %d %x %d\n",r0,r1,r2);
			result=read_syscall(r0,(char *)r1,(size_t)r2);
			break;

		case SYSCALL_WRITE:	/* 4 */
//			printk("Trying to write: %d %x %d\n",
//				r0,r1,r2);
			result=write_syscall(r0,(char *)r1,(size_t)r2);
			break;

		case SYSCALL_OPEN:	/* 5 */
			result=open_syscall((char *)r0,r1,r2);
			break;

		case SYSCALL_CLOSE:	/* 6 */
			result=close_syscall(r0);
			break;

		case SYSCALL_WAITPID:	/* 7 */
			//printk("Trying to waitpid on pid %d\n",r0);
			result=waitpid(r0,(int32_t *)r1,r2,current_proc[get_cpu()]);
			break;

		case SYSCALL_UNLINK:	/* 10 */
			result=unlink_syscall((char *)r0);
			break;

		case SYSCALL_EXECVE:	/* 11 */
			//printk("Trying to exec %s\n",(char *)r0);
			result=execve((char *)r0,(char **)r1,(char **)r2);
			/* wake up our parent (why would we do that???)*/
			/* oh, because separate process now, I see */
			//printk("Waking parent %d\n",current_proc[get_cpu()]>parent->pid);
			current_proc[get_cpu()]->parent->status=PROCESS_STATUS_READY;
			schedule();

			/* Note, we set result to value of r0 from execve */
			/* argv, otherwise it gets overwritten when we start */
			break;

		case SYSCALL_CHDIR:	/* 12 */
			result=chdir_syscall((char *)r0);
			break;

		case SYSCALL_TIME:	/* 13 */
			result=time_since_boot();
			break;

		case SYSCALL_CHMOD:	/* 15 */
			result=chmod_syscall((char *)r0,r1);
			break;

		case SYSCALL_GETPID:	/* 20 */
			result=current_proc[get_cpu()]->pid;
			break;

		case SYSCALL_TIMES:	/* 43 */
			result=times((struct tms *)r0);
			break;

		case SYSCALL_IOCTL:	/* 54 */
			result=ioctl_syscall(r0,r1,r2,r3);
			break;

		case SYSCALL_FCNTL:	/* 55 */
			result=fcntl_syscall(r0,r1,r2);
			break;

		case SYSCALL_DUP2:	/* 63 */
			result=dup2_syscall(r0,r1);
			break;

		case SYSCALL_MMAP:	/* 90 */
			result=-ENOSYS;
			break;

		case SYSCALL_MUNMAP:	/* 91 */
			result=-ENOSYS;
			break;

		case SYSCALL_STATFS:	/* 99 */
			result=statfs_syscall((const char *)r0,
						(struct vmwos_statfs *)r1);
			break;

		case SYSCALL_DMESG:	/* 103 */
			result=dmesg_syscall(r0,(char *)r1);
			break;

		case SYSCALL_STAT:	/* 106 */
			result=stat_syscall((char *)r0,
					(struct vmwos_stat *)r1);
			break;

		case SYSCALL_SYSINFO:	/* 116 */
			result=sysinfo((struct sysinfo *)r0);
			break;

		case SYSCALL_UNAME:	/* 122 */
			result=uname((struct utsname *)r0);
			break;

		case SYSCALL_LLSEEK:	/* 140 */
			result64=llseek_syscall(r0,((int64_t)r1)<<32|r2,r4);
			if (result64>=0) {
				*((uint64_t *)r3)=result64;
				result=0;
			}
			else {
				result=result64;
			}
			break;

		case SYSCALL_GETDENTS:	/* 141 */
			result=getdents_syscall(r0,
					(struct vmwos_dirent *)r1,r2);
			break;

		case SYSCALL_NANOSLEEP:	/* 162 */
			result=nanosleep((struct timespec *)r0,
					(struct timespec *)r1);
			break;

		case SYSCALL_GETCWD:	/* 183 */
			result=(uint32_t)getcwd_syscall((char *)r0,r1);
			break;

		case SYSCALL_VFORK:	/* 190 */
			//printk("Trying to vfork\n");
			result=vfork();
			break;

		case SYSCALL_TRUNCATE64:	/* 193 */
			result=truncate64_syscall((char *)r0,
							((uint64_t)r3)<<32|r2);
			break;

		case SYSCALL_FTRUNCATE64:	/* 194 */
			result=ftruncate64_syscall(r0,((uint64_t)r3)<<32|r2);
			break;

		case SYSCALL_CLOCK_GETTIME:	/* 263 */
			result=clock_gettime(r0,(struct timespec *)r1);
			break;

		case SYSCALL_GETCPU:		/* 345 */
			result=getcpu((uint32_t *)r0,
					(uint32_t *)r1,
					(void *)r2);
			break;

		/******************/
		/* VMWOS SPECIFIC */
		/******************/


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
			result=framebuffer_gradient(r0);
			break;

		case SYSCALL_FRAMEBUFFER_LOAD:
			result=framebuffer_load(r0,r1,r2,(char *)r3);
			break;

		case SYSCALL_REBOOT:
			/* See https://www.raspberrypi.org/forums/viewtopic.php?f=72&t=53862 */
			bcm2835_write(PM_WDOG, PM_PASSWORD | 1);	/* timeout = 1/16th of a second? */
			bcm2835_write(PM_RSTC, PM_PASSWORD | PM_RSTC_WRCFG_FULL_RESET);
			result = 0;
			break;


		case SYSCALL_TEMPERATURE:
			result=thermal_read();
			break;

		case SYSCALL_RANDOM:
			result=bcm2835_rng_read((uint32_t *)r0);
			break;

		case SYSCALL_MALLOC:
			result=(uint32_t)memory_allocate(r0,r1);
			break;

		case SYSCALL_PLAY_SOUND:
			result=(uint32_t)audio_pwm_write((uint32_t *)r0,r1,r2);
			break;

		case SYSCALL_CORE_POKE:
			result=send_ipi(r0);
			break;

		default:
			printk("Unknown syscall %d\n",r7);
			break;
	}

	return result;

}

