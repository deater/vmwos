#define DEFAULT_STACK_SIZE 8192
#define MAX_PROCESSES 10

#define PROCESS_FROM_DISK	1
#define PROCESS_FROM_RAM	2

int load_process(char *name, int type, char *data, int size, int stacksize);
void schedule(long *pcb);
int run_process(int which,long irq_stack);
int processes_init(void);

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
