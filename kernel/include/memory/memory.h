//extern unsigned int memory_total;

//int memory_init(unsigned long memory_total,unsigned long memory_kernel);
void memory_hierarchy_init(unsigned long memory_kernel);

void *memory_allocate(uint32_t size);
int32_t memory_free(void *location, uint32_t size);
int32_t memory_total_free(void);
uint32_t memory_get_total(void);
