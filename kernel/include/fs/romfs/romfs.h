struct romfs_header_t {
	char magic[8];
	int size;
	int checksum;
	char volume_name[17];
	int first_offset;
};

struct romfs_file_header_t {
	int addr;
	int next;
	int type;
	int special;
	int size;
	int checksum;
	int filename_start;
	int data_start;
};

int32_t romfs_mount(struct superblock_type *superblock,
		struct block_dev_type *block);

#if 0
int32_t open_romfs_file(char *name,
		struct romfs_file_header_t *file);
int32_t romfs_get_inode(int32_t inode_number, struct inode_type *inode);


int32_t romfs_read_inode(struct inode_type *inode);
int32_t romfs_lookup_inode(struct inode_type *dir_inode, const char *name);

int32_t romfs_getdents(struct superblock_type *superblock,
		uint32_t dir_inode,
		uint64_t *current_progress, void *buf,uint32_t size);
int32_t romfs_statfs(struct superblock_type *superblock,struct vmwos_statfs *buf);

int32_t romfs_read_file(
			struct superblock_type *superblock, uint32_t inode,
			char *buf,uint32_t count, uint64_t *offset);
int32_t romfs_write_file(struct superblock_type *superblock, uint32_t inode,
			const char *buf,uint32_t count, uint64_t *offset);

#endif

#define ROMFS_TYPE_HARDLINK	0
#define ROMFS_TYPE_DIRECTORY	1
#define ROMFS_TYPE_REGULARFILE	2
#define ROMFS_TYPE_SYMBOLICLINK	3
#define ROMFS_TYPE_BLOCKDEV	4
#define ROMFS_TYPE_CHARDEV	5
#define ROMFS_TYPE_SOCKET	6
#define ROMFS_TYPE_FIFO		7


