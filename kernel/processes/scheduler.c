#include <stddef.h>
#include <stdint.h>

#include "syscalls/exec.h"
#include "memory/memory.h"
#include "processes/scheduler.h"
#include "processes/process.h"
#include "time/time.h"

#include "lib/printk.h"
#include "lib/string.h"
#include "lib/memcpy.h"

int scheduling_enabled=1;

static int schedule_debug=0;

void schedule(void) {

	struct process_control_block_type *proc,*orig_proc;

	orig_proc=current_process;
	proc=current_process;

	if (schedule_debug)
		printk("Attempting to schedule, current proc=%d (%x)\n",
			proc->pid,(long)proc);

	/* find next available process */

	/* Special case if in idle thread */
	if (proc==proc_first) {
		if (proc->next==NULL) return;
		proc=proc->next;
		orig_proc=proc;
	}

	while(1) {
		if (schedule_debug) {
			printk("proc=%x, proc->next=%x\n",
					(long)proc,(long)proc->next);
		}

		proc=proc->next;
		if (schedule_debug) {
			printk("What about proc->next %x?\n",(long)proc);
		}

		/* wrap around if off end */
		if (proc==NULL) {
//			if (schedule_debug)
//				printk("proc->next is NULL, wrapping\n");
			proc=proc_first->next;
			if (proc==NULL) {
				proc=proc_first;
//				if (schedule_debug)
//					printk("scheduler: only %d available\n",
//						proc->pid);
				break;
			}
//			if (schedule_debug)
//				printk("scheduler: wrapping to %d\n",
//					proc->pid);
		}

		/* if valid and ready, then run it */
		if (proc->status==PROCESS_STATUS_READY) {
//			if (schedule_debug)
//				printk("scheduler: process %d looks ready\n",proc->pid);
			break;
		}
		else {
//			if (schedule_debug)
//				printk("SCHEDULER: process %d not ready\n",proc->pid);
		}

		/* Nothing was ready, run idle task */
		if (proc==orig_proc) {
			proc=proc_first;
//			if (schedule_debug)
//				printk("SCHEDULER: running idle task %d\n",proc->pid);
			break;
		}

	}


	/* Update time stats */
	current_process->total_time+=(ticks_since_boot()-current_process->last_scheduled);
	proc->last_scheduled=ticks_since_boot();

	/* switch to new process */
	if (proc!=current_process) {
		if (schedule_debug)
			printk("SCHEDULER: Switching from proc %d (%x) to %d (%x)\n",
				current_process->pid,(long)current_process,
				proc->pid,(long)proc);
		process_switch(current_process,proc);
	}

	if (schedule_debug) {
		printk("SCHEDULER: Sticking with proc %d\n",
			current_process->pid);
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
