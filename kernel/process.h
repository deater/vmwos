#define DEFAULT_USER_STACK_SIZE		8192
#define DEFAULT_KERNEL_STACK_SIZE	4096
#define MAX_PROCESSES 10

#define PROCESS_FROM_DISK	1
#define PROCESS_FROM_RAM	2

#define PROCESS_STATUS_SLEEPING	0
#define PROCESS_STATUS_READY	1
#define PROCESS_STATUS_EXITED	2

extern struct process_control_block_type *proc_first;

struct process_control_block_type {
	struct process_control_block_type *next;	/*   0 */
	struct process_control_block_type *prev;	/*   4 */
	int32_t valid;					/*   8 */
	int32_t running;				/*  16 */
	int32_t status;					/*  20 */
	int32_t time;					/*  24 */
	int32_t pid;					/*  28 */
	int32_t exit_value;				/*  32 */
	struct process_control_block_type *parent;	/*  36 */

	struct {
		uint32_t r[15];	/* r0 - r14 */		/*  40 */
		uint32_t lr,spsr;			/* 100 */
	} reg_state;
	void *stack;					/* 108 */
	uint32_t stacksize;				/* 112 */
	void *text;					/* 116 */
	uint32_t textsize;				/* 120 */
	char name[32];					/* 124 */
							/* 132 */
	/* Current size = 132 */

	uint32_t stack_padding[DEFAULT_KERNEL_STACK_SIZE-132];

};

extern int userspace_started;
extern struct process_control_block_type *current_process;

//int32_t process_table_init(void);
//int32_t process_load(char *name, int type, char *data, int size, int stack_size);
int32_t process_run(struct process_control_block_type *proc, long *irq_stack);
int32_t process_save(struct process_control_block_type *proc, long *irq_stack);

struct process_control_block_type *process_create(void);
int32_t process_destroy(struct process_control_block_type *proc);

struct process_control_block_type *process_lookup(int32_t pid);
