#define DEFAULT_STACK_SIZE 8192
#define MAX_PROCESSES 10

#define PROCESS_FROM_DISK	1
#define PROCESS_FROM_RAM	2

struct process_control_block_type {
	int valid;
	int running;
	int ready;
	int time;
	int pid;
	char name[32];

	struct {
		long r[15];	/* r0 - r14 */
		long lr,spsr;
	} reg_state;
};

extern int current_process;
extern int userspace_started;
extern struct process_control_block_type process[];

int32_t process_table_init(void);
int32_t process_load(char *name, int type, char *data, int size, int stack_size);
int32_t process_run(int which, long irq_stack);
int32_t process_save(int which, long *pcb);

