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
int32_t romfs_get_inode(const char *name);
int32_t romfs_read_file(uint32_t inode, uint32_t offset,
			void *buf,uint32_t count);
int32_t romfs_mount(void);
int32_t romfs_stat(int32_t inode, struct stat *buf);

#define ROMFS_TYPE_HARDLINK	0
#define ROMFS_TYPE_DIRECTORY	1
#define ROMFS_TYPE_REGULARFILE	2
#define ROMFS_TYPE_SYMBOLICLINK	3
#define ROMFS_TYPE_BLOCKDEV	4
#define ROMFS_TYPE_CHARDEV	5
#define ROMFS_TYPE_SOCKET	6
#define ROMFS_TYPE_FIFO		7


