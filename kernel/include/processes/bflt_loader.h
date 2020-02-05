int32_t bflt_load(struct file_object *file,
                uint32_t *stack_size, uint32_t *text_start,
                uint32_t *data_start, uint32_t *bss_start,
                uint32_t *bss_end,
		uint32_t *total_ondisk_size,
                uint32_t *total_program_size);

int32_t bflt_reloc(struct file_object *file, void *binary_address);

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

#define BFLT_FLAG_RAM		0x0001 /* load program entirely into RAM */
#define BFLT_FLAG_GOTPIC	0x0002 /* program is PIC with GOT */
#define BFLT_FLAG_GZIP		0x0004 /* all but the header is compressed */
#define BFLT_FLAG_GZDATA	0x0008 /* only data/relocs compressed (XIP) */
#define BFLT_FLAG_KTRACE	0x0010 /* ktrace debugging (not implemented) */
#define BFLT_FLAG_L1STK		0x0020 /* 4k stack in L1 (not imp)  */

