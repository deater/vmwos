#define DEFAULT_STACK_SIZE 8192
#define MAX_PROCESSES 10

#define PROCESS_FROM_DISK	1
#define PROCESS_FROM_RAM	2

#define PROCESS_STATUS_SLEEPING	0
#define PROCESS_STATUS_READY	1
#define PROCESS_STATUS_EXITED	2

struct process_control_block_type {
	int32_t valid;
	int32_t running;
	int32_t status;
	int32_t time;
	int32_t pid;
	int32_t exit_value;
	char name[32];

	struct {
		long r[15];	/* r0 - r14 */
		long lr,spsr;
	} reg_state;
	void *stack;
	uint32_t stacksize;
	void *text;
	uint32_t textsize;
};

extern int current_process;
extern int userspace_started;
extern struct process_control_block_type process[];

int32_t process_table_init(void);
int32_t process_load(char *name, int type, char *data, int size, int stack_size);
int32_t process_run(int which, long irq_stack);
int32_t process_save(int which, long *pcb);

int32_t process_create(void);
int32_t process_destroy(int32_t which);
