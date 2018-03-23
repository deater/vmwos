#define DEFAULT_USER_STACK_SIZE		8192
#define DEFAULT_KERNEL_STACK_SIZE	8192
#define MAX_PROCESSES 10

#define PROCESS_FROM_DISK	1
#define PROCESS_FROM_RAM	2

#define PROCESS_STATUS_SLEEPING	0
#define PROCESS_STATUS_READY	1
#define PROCESS_STATUS_EXITED	2

extern struct process_control_block_type *proc_first;

struct process_control_block_type {
	struct {
		uint32_t r[15];	/* r0 - r14 */		/*   0 */
		uint32_t pc;				/*  60 */
		uint32_t spsr;				/*  64 */
	} user_state;
	struct {
		uint32_t r[15];	/* r0 - r14 */		/*  68 */
		uint32_t spsr;				/* 128 */
	} kernel_state;

	struct process_control_block_type *next;	/* 132 */
	struct process_control_block_type *prev;	/* 136 */
	struct process_control_block_type *wait_queue_next;	/* 140 */
	int32_t valid;					/* 144 */
	int32_t running;				/* 148 */
	int32_t status;					/* 152 */
	int32_t start_time;				/* 156 */
	int32_t last_scheduled;				/* 160 */
	int32_t total_time;				/* 164 */
	int32_t pid;					/* 168 */
	int32_t exit_value;				/* 172 */
	struct process_control_block_type *parent;	/* 176 */
	int32_t current_dir;				/* 180 */

	void *stack;					/* 184 */
	uint32_t stacksize;				/* 188 */
	void *text;					/* 192 */
	uint32_t textsize;				/* 196 */
	void *data;					/* 200 */
	uint32_t datasize;				/* 204 */
	void *bss;					/* 208 */
	uint32_t bsssize;				/* 212 */
	char name[32];					/* 216 */
							/* 248 */
	/* Current size = 248 */

	uint32_t stack_padding[(DEFAULT_KERNEL_STACK_SIZE-248)/4];

};

extern struct process_control_block_type *current_process;

//int32_t process_table_init(void);
//int32_t process_load(char *name, int type, char *data, int size, int stack_size);
int32_t process_run(struct process_control_block_type *proc);
void process_save(struct process_control_block_type *proc,
		uint32_t new_stack);

struct process_control_block_type *process_create(void);
int32_t process_destroy(struct process_control_block_type *proc);

struct process_control_block_type *process_lookup(int32_t pid);
struct process_control_block_type *process_lookup_child(
        struct process_control_block_type *caller);


int32_t process_switch(struct process_control_block_type *old,
		struct process_control_block_type *new);

int32_t process_get_totals(int32_t type, int32_t *count);
