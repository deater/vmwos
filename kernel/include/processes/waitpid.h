#define WNOHANG		0x00000001
#define WUNTRACED	0x00000002
#define WSTOPPED	WUNTRACED
#define WEXITED		0x00000004
#define WCONTINUED	0x00000008
#define WNOWAIT		0x01000000

int32_t waitpid_done(void);
int32_t waitpid(int32_t pid, int32_t *wstatus, int32_t options,
	struct process_control_block_type *caller);




