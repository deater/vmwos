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

int romfs_read(void *buffer, uint32_t *offset, uint32_t size);
int open_romfs_file(char *name,
		struct romfs_file_header_t *file);
uint32_t romfs_get_inode(const char *name);
uint32_t romfs_read_file(uint32_t inode, uint32_t offset,
			void *buf,uint32_t count);
uint32_t romfs_mount(void);
