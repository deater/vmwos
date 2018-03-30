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

int32_t open_romfs_file(char *name,
		struct romfs_file_header_t *file);
int32_t romfs_get_inode(int32_t dir_inode, const char *name);
int32_t romfs_read_file(uint32_t inode, uint32_t offset,
			void *buf,uint32_t count);
int32_t romfs_mount(struct superblock_t *superblock);
int32_t romfs_stat(int32_t inode, struct stat *buf);
int32_t romfs_getdents(uint32_t dir_inode,
		uint32_t *current_inode, void *buf,uint32_t size);
int32_t romfs_statfs(struct superblock_t *superblock,struct statfs *buf);

#define ROMFS_TYPE_HARDLINK	0
#define ROMFS_TYPE_DIRECTORY	1
#define ROMFS_TYPE_REGULARFILE	2
#define ROMFS_TYPE_SYMBOLICLINK	3
#define ROMFS_TYPE_BLOCKDEV	4
#define ROMFS_TYPE_CHARDEV	5
#define ROMFS_TYPE_SOCKET	6
#define ROMFS_TYPE_FIFO		7


