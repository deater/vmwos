#define BLOCK_NAME_LENGTH	32
#define BLOCK_DEV_MAX		8

struct block_dev_type;

struct block_operations {
	int32_t (*read) (struct block_dev_type *,uint32_t, uint32_t, char *);
	int32_t (*write) (struct block_dev_type *,uint32_t, uint32_t, char *);
	int32_t (*ioctl) (struct block_dev_type *,uint32_t, uint32_t, uint32_t);
//        int32_t (*open) (int32_t *, struct file_object *);
};


struct block_dev_type {
	int32_t major;
	int32_t minor;
	char name[BLOCK_NAME_LENGTH];
	uint64_t start,length;
	uint64_t capacity;
	struct block_operations *block_ops;
	void *private;
};

struct block_dev_type *allocate_block_dev(void);

struct block_dev_type *block_dev_find(const char *name);
struct block_dev_type *block_dev_lookup(uint32_t rdev);


