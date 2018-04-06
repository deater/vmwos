void enable_mmu(uint32_t mem_start, uint32_t mem_end, uint32_t kernel_end);
void enable_l1_dcache(void);
void disable_l1_dcache(void);
void enable_l1_icache(void);
void disable_l1_icache(void);
void enable_branch_predictor(void);
void disable_branch_predictor(void);

void flush_icache(void);
void flush_dcache(uint32_t start_addr, uint32_t end_addr);
