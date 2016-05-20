/* Waitqueues, for blocking I/O */

/* Some useful references on how Linux does this */

/*
http://www.tldp.org/LDP/lki/lki-2.html
https://lwn.net/Articles/577370/
http://www.makelinux.net/ldd3/chp-6-sect-2
*/

#include <stdint.h>
#include <stddef.h>

#include "process.h"
#include "scheduler.h"
#include "wait.h"

int32_t wait_queue_add(struct wait_queue_t *queue, struct process_control_block_type *proc) {

	struct process_control_block_type *next;

	if (queue->first==NULL) {
		queue->first=proc;
	}
	else {
		next=queue->first;
		while(1) {
			if (next->wait_queue_next==NULL) {
				next->wait_queue_next=proc;
				break;
			}
			next=next->wait_queue_next;
		}
	}
	proc->status=PROCESS_STATUS_SLEEPING;
	proc->wait_queue_next=NULL;
	schedule();

	return 0;
}

int32_t wait_queue_wake(struct wait_queue_t *queue) {

	struct process_control_block_type *proc;

	proc=queue->first;

	while(proc!=NULL) {
		proc->status=PROCESS_STATUS_READY;
		proc=proc->wait_queue_next;
	}
	queue->first=NULL;

	return 0;
}

