#include <stddef.h>
#include <stdint.h>

#include "exec.h"
#include "memory.h"
#include "scheduler.h"
#include "process.h"
#include "time.h"

#include "lib/printk.h"
#include "lib/string.h"
#include "lib/memcpy.h"

int scheduling_enabled=1;

void schedule(void) {

	struct process_control_block_type *proc,*orig_proc;

	orig_proc=current_process;
	proc=current_process;

//	printk("Attempting to schedule, current proc=%d (%x)\n",
//		proc->pid,(long)proc);

	/* find next available process */
	/* Should we have an idle process (process 0) */
	/* That is special cased and just runs wfi?   */

	/* Special case if in idle thread */
	if (proc==proc_first) {
		if (proc->next==NULL) return;
		proc=proc->next;
		orig_proc=proc;
	}

	while(1) {
//		printk("proc=%x, proc->next=%x\n",(long)proc,(long)proc->next);
		proc=proc->next;
//		printk("What about proc->next %x?\n",(long)proc);


		/* wrap around if off end */
		if (proc==NULL) {
//			printk("proc->next is NULL, wrapping\n");
			proc=proc_first->next;
			if (proc==NULL) {
				proc=proc_first;
//				printk("scheduler: only %d available\n",
//					proc->pid);
				break;
			}
//			printk("scheduler: wrapping to %d\n",
//				proc->pid);
		}

		/* if valid and ready, then run it */
		if (proc->status==PROCESS_STATUS_READY) {
//			printk("scheduler: process %d looks ready\n",proc->pid);
			break;
		}
		else {
//			printk("scheduler: process %d not ready\n",proc->pid);
		}

		/* Nothing was ready, run idle task */
		if (proc==orig_proc) {
			proc=proc_first;
//			printk("scheduler: giving up and running %d\n",proc->pid);
			break;
		}

	}


	/* Update time stats */
	current_process->total_time+=(ticks_since_boot()-current_process->last_scheduled);
	proc->last_scheduled=ticks_since_boot();

	/* switch to new process */
	if (proc!=current_process) {
//		printk("Switching from %d (%x) to %d (%x)\n",
//			current_process->pid,(long)current_process,
//			proc->pid,(long)proc);
		process_switch(current_process,proc);
	}

	/* ARM documentation says we can put a */
	/* clrex instruction here */
	/* to avoid false positives in the mutex handling? */
	/* if there's any aliasing between new and old process? */

}

int32_t sched_yield(void) {

	schedule();

	return 0;

}
