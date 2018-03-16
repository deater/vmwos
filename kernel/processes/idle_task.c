#include <stdint.h>

#include "lib/printk.h"
#include "lib/delay.h"
#include "lib/string.h"

#include "processes/process.h"
#include "processes/idle_task.h"
#include "processes/userspace.h"
#include "memory/memory.h"
#include "time/time.h"

static void idle_task(void) {

//	printk("Idle Task!\n");

	while(1) {
//		printk("Idle\n");
//		delay(1000);

		/* Interesting, see */
		/* http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.faqs/ka13332.html */
		/* On ARM1176 wfi is a nop */
		/* Use the traditional MCR p15, 0, Rn, c7, c0, 4 instead */
		/* Rn value SBZ should be zero */
		/* Works by having the interrupt handler return +8 instead */
		/* of +4 */
//		asm volatile("MCR p15, 0, %0, c7, c0, 4\n" ::"r"(0):);
		asm volatile("wfi\n" :::);

	}

//	asm volatile("idle_loop:\n"
//			"wfi\n"
//			"b idle_loop\n"
//			:::);

//	asm volatile("idle_loop:\n"
//			"nop\n"
//			"b idle_loop\n"
//			:::);
}



void create_idle_task(void) {

	struct process_control_block_type *idle_process;

	idle_process=process_create();
	idle_process->text=(void *)&idle_task;
	idle_process->user_state.pc=(long)&idle_task;
	idle_process->running=1;
	idle_process->total_time=0;
	idle_process->start_time=ticks_since_boot();
	idle_process->last_scheduled=idle_process->start_time;
	idle_process->stack=memory_allocate(4096);
	/* Point to end, as stack grows down */
	idle_process->user_state.r[13]=(long)(idle_process->stack)+4096;

	strncpy(idle_process->name,"idle",5);
	idle_process->kernel_state.r[14]=(long)enter_userspace;

	idle_process->status=PROCESS_STATUS_READY;

	printk("Created idle thread: %d\n",idle_process->pid);
}


