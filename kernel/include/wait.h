/* Waitqueues, for blocking I/O */

struct wait_queue_t {
	struct process_control_block_type *first;
};

int32_t wait_queue_add(struct wait_queue_t *queue, struct process_control_block_type *proc);
int32_t wait_queue_wake(struct wait_queue_t *queue);
