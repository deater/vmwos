#ifdef ARMV7
#define NUM_CORES	4
#else
#define NUM_CORES	1
#endif

int32_t get_cpu(void);
