#include <stddef.h>
#include <stdint.h>

#include "lib/printk.h"
#include "lib/string.h"
#include "lib/memcpy.h"
#include "lib/memset.h"
#include "lib/smp.h"

#include "syscalls/exec.h"
#include "memory/memory.h"
#include "processes/process.h"
#include "time/time.h"
#include "fs/files.h"

static int process_debug=0;

int userspace_started=0;
static int avail_pid=0;

struct process_control_block_type *proc_first=NULL;
//struct process_control_block_type *current_process=NULL;

struct process_control_block_type *current_proc[NUM_CORES];

#define MAX_PROCS	16
static struct process_control_block_type __attribute__((aligned(4096)))
	all_processes[MAX_PROCS];



int32_t process_get_totals(int32_t type, int32_t *count) {

	int32_t total_count=0;
	struct process_control_block_type *proc_ptr=proc_first;

	*count=0;

	while(1) {
		total_count++;
		if (proc_ptr->status==PROCESS_STATUS_READY) (*count)++;
		proc_ptr=proc_ptr->next;
		if (proc_ptr==NULL) break;
	}

	return total_count;
}

struct process_control_block_type *process_lookup(int32_t pid) {

	struct process_control_block_type *result=NULL;

	result=proc_first;
	while(result!=NULL) {
		if (result->pid==pid) return result;

		result=result->next;
	}

	return result;

}

struct process_control_block_type *process_lookup_child(
	struct process_control_block_type *caller) {

	struct process_control_block_type *result=NULL;

	result=proc_first;
	while(result!=NULL) {

		/* Init is own parent? */
		if ((result!=caller) && (result->parent==caller)) {
			return result;
		}

		result=result->next;
	}

	return result;

}

/* Insert process into linked list */
static int32_t process_insert(struct process_control_block_type *proc) {

	struct process_control_block_type *last;

	if (proc_first==NULL) {
		if (process_debug) {
			printk("Creating first process %d\n",proc->pid);
		}
		proc_first=proc;
		proc->next=NULL;
		proc->prev=NULL;
		return 0;
	}

	last=proc_first;
	while(last->next!=NULL) {
		last=last->next;
	}
	last->next=proc;
	proc->next=NULL;
	proc->prev=last;
	if (process_debug) {
		printk("Putting new process %d after %d\n",proc->pid,last->pid);
	}
//	printk("proc %x proc->next %x proc->prev %x\n",
//		(long)proc,(long)proc->next,(long)proc->prev);

	return 0;
}


/* Delete process from linked list */
static int32_t process_remove(struct process_control_block_type *proc) {

	struct process_control_block_type *current;

	if (proc_first==NULL) {
		printk("Attempting to delete from empty list!\n");
		return -1;
	}

	if (proc_first==proc) {
		printk("Deleting first process!\n");
		proc_first=proc->next;
		return 0;
	}

	current=proc_first;

	while(current!=NULL) {
		if (current==proc) {
			current->prev->next=proc->next;
			if (proc->next!=NULL) {
				proc->next->prev=proc->prev;
			}
			return 0;
		}
		current=current->next;
	}
	printk("process_remove: ERROR could not remove!\n");

	return 0;
}


struct process_control_block_type *process_create(void) {

	int i;
	struct process_control_block_type *new_proc=NULL;

	for(i=0;i<MAX_PROCS;i++) {
		if (all_processes[i].valid==0) {
			new_proc=&all_processes[i];
			break;
		}
	}

//	new_proc=memory_allocate(sizeof(struct process_control_block_type));
	if (new_proc==NULL) {
		printk("process_create: out of memory\n");
		return NULL;
	}

	/* clear to zero */
	memset(new_proc,0,sizeof(struct process_control_block_type));

	/* Mark as valid */
	new_proc->valid=1;

	if (process_debug) {
		printk("process_create: allocated %d bytes for PCB at %x\n",
			sizeof(struct process_control_block_type),
			(long)new_proc);
	}

	/* Set up initial conditions */
	new_proc->running=0;
	new_proc->status=PROCESS_STATUS_SLEEPING;

	/* Set up process time accounting */
	new_proc->start_time=ticks_since_boot();
	new_proc->last_scheduled=new_proc->start_time;
	new_proc->user_time=0;
	new_proc->kernel_time=0;

	/* Set up file descriptors */
	for(i=0;i<MAX_FD_PER_PROC;i++) new_proc->files[i]=NULL;

	/* LOCK */
	/* FIXME: what happens when we rollover */
	new_proc->pid=avail_pid;
	avail_pid++;
	/* UNLOCK */

	new_proc->stack=NULL;
	new_proc->text=NULL;
	new_proc->stacksize=0;
	new_proc->textsize=0;

	/* Clear out registers */
	for(i=0;i<14;i++) new_proc->user_state.r[i]=0;

	/* Setup the default SPSR */
	/* USER mode (0x10) */
	/* We don't mask 0x80 or 0x40 (IRQ or FIQ) */
	new_proc->user_state.spsr=0x10;

	/* Clear out LR (saved pc) */
	/* exec should set this for us */

	/* Should point this to PANIC in case init or idle_task exit? */
	new_proc->user_state.r[14]=0;

	/* Insert into linked list */
	process_insert(new_proc);

	return new_proc;
}

int32_t process_destroy(struct process_control_block_type *proc) {

	int i;

	if (process_debug) {
		printk("ATTEMPTING TO DESTROY PROCESS %d\n",proc->pid);
	}

	/* mark as no longer valid */
	proc->valid=0;

	/* remove from linked list? */
	process_remove(proc);

	/* close open files */
	for(i=0;i<MAX_FD_PER_PROC;i++) {
		if (proc->files[i]!=NULL) file_object_free(proc->files[i]);
	}

	/* free memory */
	if (proc->stack) {
		memory_free(proc->stack,proc->stacksize);
	}
	if (proc->text) {
		memory_free(proc->text,proc->textsize);
	}

	/* Delete proc struct itself */
	/* Not needed anymore */
//	memory_free(proc,sizeof(struct process_control_block_type));

	return 0;
}

void process_save(struct process_control_block_type *proc, uint32_t new_stack) {
	/* Save current state to PCB */
	asm(
		"mov    r2, %[save]\n"
		"stmia  r2,{r0-lr}\n"	//Save all registers r0-lr
		"add	r2,r2,#52\n"	// point to r13 save space
		"str	%[new_stack],[r2]\n" // store new stack
		"add    r2,r2,#8\n"
		"mrs    r0, SPSR\n"	// load SPSR
		"stmia  r2,{r0}\n"	// store
		: /* output */
		: [save] "r"(&(proc->kernel_state.r[0])) ,
		  [new_stack] "r"(new_stack)
		  /* input */
		: /* clobbers */
	);

}

int32_t process_switch(struct process_control_block_type *old,
			struct process_control_block_type *new) {

        /* Save current state to PCB */
        asm(
                "mov    r2, %[save]\n"
                "stmia  r2,{r0-lr}\n"   //Save all registers r0-lr
                "add    r2,r2,#60\n"
                "mrs    r0, SPSR\n"     //  load SPSR
                "stmia  r2,{r0}\n"      // store
                : /* output */
                : [save] "r"(&(old->kernel_state.r[0]))/* input */
                : /* clobbers */
        );

	current_proc[0]=new;

        /* Restore current state from PCB */
        asm(
                "mov    r2, %[restore]\n"
		"ldr	r0,[r2,#60]\n"
		"msr	SPSR, r0\n"		// restore SPSR
                "ldmia	r2,{r0-r14}\n"	// restore registers
                "mov	pc,lr\n"	// return, restoring SPSR
                : /* output */
                : [restore] "r"(&(new->kernel_state.r[0]))
		/* input */
                : /* clobbers */
        );

	return 0;
}


