#define DOS33_SUPER_MAGIC	0x02131978
#define DOS33_BLOCK_SIZE	256
#define DOS33_MAX_FILENAME_SIZE	30
#define DOS33_VTOC_SIZE		256
#define DOS33_MAX_VTOCS		2


#define DOS33_VTOC_VOLUME	0x06
#define DOS33_VTOC_NUM_TRACKS	0x34
#define DOS33_VTOC_NUM_SECTORS	0x35
#define DOS33_VTOC_FREE_BITMAPS	0x38

int32_t dos33fs_mount(struct superblock_type *superblock,
		struct block_dev_type *block);



