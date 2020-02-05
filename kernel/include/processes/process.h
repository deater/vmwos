#define DEFAULT_USER_STACK_SIZE		8192
#define DEFAULT_KERNEL_STACK_SIZE	8192
#define MAX_PROCESSES 10

#define PROCESS_FROM_DISK	1
#define PROCESS_FROM_RAM	2

#define PROCESS_STATUS_SLEEPING	0
#define PROCESS_STATUS_READY	1
#define PROCESS_STATUS_EXITED	2

/* Also defined in files.h */
#define MAX_FD_PER_PROC	8
#define	MAX_PATH_LEN	1024


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

	/* Time Accounting */
	uint32_t start_time;				/* 156 */
	uint32_t user_time;				/* 160 */
	uint32_t kernel_time;				/* 164 */
	int32_t last_scheduled;				/* 168 */

	int32_t pid;					/* 172 */
	int32_t exit_value;				/* 176 */
	struct process_control_block_type *parent;	/* 180 */

	/* Memory Layout */
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

	/* File related things */
	struct file_object *files[MAX_FD_PER_PROC];	/* 8*4 = 32 */	/* 280 */
	char current_dir[MAX_PATH_LEN];	/* 1024 */	/*1304 */

	/* Current size = 1304 */

	uint32_t stack_padding[(DEFAULT_KERNEL_STACK_SIZE-1304)/4];

};

extern struct process_control_block_type *current_proc[NUM_CORES];

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
