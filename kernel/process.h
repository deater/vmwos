#define DEFAULT_USER_STACK_SIZE		8192
#define DEFAULT_KERNEL_STACK_SIZE	4096
#define MAX_PROCESSES 10

#define PROCESS_FROM_DISK	1
#define PROCESS_FROM_RAM	2

#define PROCESS_STATUS_SLEEPING	0
#define PROCESS_STATUS_READY	1
#define PROCESS_STATUS_EXITED	2

struct process_control_block_type {
	struct process_control_block_type *next;	/*   0 */
	int32_t valid;					/*   4 */
	int32_t running;				/*   8 */
	int32_t status;					/*  16 */
	int32_t time;					/*  20 */
	int32_t pid;					/*  24 */
	int32_t exit_value;				/*  28 */
	int32_t parent;					/*  32 */

	struct {
		uint32_t r[15];	/* r0 - r14 */		/*  36 */
		uint32_t lr,spsr;			/*  96 */
	} reg_state;
	void *stack;					/* 104 */
	uint32_t stacksize;				/* 108 */
	void *text;					/* 112 */
	uint32_t textsize;				/* 116 */
	char name[32];					/* 120 */
							/* 128 */
	/* Current size = 128 */

	uint32_t stack_padding[DEFAULT_KERNEL_STACK_SIZE-128];

};

extern int current_process;
extern int userspace_started;
extern struct process_control_block_type process[];

int32_t process_table_init(void);
int32_t process_load(char *name, int type, char *data, int size, int stack_size);
int32_t process_run(int which, long *irq_stack);
int32_t process_save(int which, long *irq_stack);

int32_t process_create(void);
int32_t process_destroy(int32_t which);
