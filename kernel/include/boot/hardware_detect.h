#define RPI_UNKNOWN		0
#define RPI_MODEL_A		1
#define RPI_MODEL_B		2
#define RPI_MODEL_APLUS		3
#define RPI_MODEL_BPLUS		4
#define RPI_COMPUTE_NODE	6
#define RPI_MODEL_2B		7
#define RPI_MODEL_ZERO		8
#define RPI_MODEL_3B		9
#define RPI_MODEL_3BPLUS	10

uint32_t hardware_detect(void *info_ptr);
void hardware_print_model(uint32_t version);
void hardware_print_commandline(void);
uint32_t hardware_get_type(void);
void hardware_get_memory(uint32_t *start, uint32_t *length);

