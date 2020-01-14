int32_t bflt_load(int32_t inode,
                uint32_t *stack_size, uint32_t *text_start,
                uint32_t *data_start, uint32_t *bss_start,
                uint32_t *bss_end, uint32_t *total_size);

int32_t bflt_reloc(int32_t inode, void *binary_address);

#define BFLT_MAGIC		"bFLT"

#define BFLT_MAGIC_OFFSET	0x00
#define BFLT_VERSION_OFFSET	0x04
#define BFLT_ENTRY		0x08
#define BFLT_DATA_START		0x0C
#define BFLT_BSS_START		0x10
#define BFLT_BSS_END		0x14
#define BFLT_STACK_SIZE		0x18
#define BFLT_RELOC_START	0x1C
#define BFLT_RELOC_COUNT	0x20
#define BFLT_FLAGS		0x24
