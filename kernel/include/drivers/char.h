#define CHAR_NAME_LENGTH	32
#define CHAR_DEV_MAX		8

struct char_dev_type;

struct char_operations {
	int32_t (*read) (struct file_object *,char *, uint32_t);
	int32_t (*write) (struct file_object *,char *, uint32_t);
	int32_t (*ioctl) (struct file_object *,uint32_t, uint32_t, uint32_t);
//        int32_t (*open) (int32_t *, struct file_object *);
};


struct char_dev_type {
	int32_t major;
	int32_t minor;
	char name[CHAR_NAME_LENGTH];
	uint64_t start,length;
	struct char_operations *char_ops;
	void *private;
};

struct char_dev_type *allocate_char_dev(void);

struct char_dev_type *char_dev_find(const char *name);
struct char_dev_type *char_dev_lookup(uint32_t rdev);


