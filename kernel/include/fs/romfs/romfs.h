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

int romfs_read(void *buffer, int *offset, int size);
int open_romfs_file(char *name,
		struct romfs_file_header_t *file);
