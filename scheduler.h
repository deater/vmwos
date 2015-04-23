#define DEFAULT_STACK_SIZE 4096
#define MAX_PROCESSES 10

int load_process(char *data, int size, unsigned int stack_size);
void schedule(void);
int run_process(int which);

extern int current_process;

struct process_control_block_type {
	int valid;
	int running;
	int ready;
	int time;
	int pid;

	struct {
		long r[14];
		long lr,spsr;
	} reg_state;
};

extern struct process_control_block_type process[MAX_PROCESSES];


