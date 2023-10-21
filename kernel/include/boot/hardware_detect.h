#define RPI_MODEL_UNKNOWN	0
#define RPI_MODEL_A		1
#define RPI_MODEL_B		2
#define RPI_MODEL_APLUS		3
#define RPI_MODEL_BPLUS		4
#define RPI_COMPUTE_MODULE1	6
#define RPI_MODEL_2B		7
#define RPI_MODEL_ZERO		8
#define RPI_MODEL_3B		9
#define RPI_MODEL_3BPLUS	10
#define	RPI_MODEL_2B_V1_2	11
#define RPI_COMPUTE_MODULE3	12
#define RPI_MODEL_ZERO_W	13
#define RPI_MODEL_ZERO_2W	14
#define RPI_MODEL_3APLUS	15
#define RPI_MODEL_4B		16

uint32_t hardware_detect(void *info_ptr);
void hardware_print_model(uint32_t version);
void hardware_print_commandline(void);
uint32_t hardware_get_type(void);
void hardware_get_memory(uint32_t *start, uint32_t *length);

void hardware_setup_vars(void);

extern uint32_t hardware_type;
extern uint32_t io_base;
