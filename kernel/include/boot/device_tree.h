//int32_t devicetree_decode(uint32_t *r2);
//void devicetree_dump(void);

#define DT_STRING_MAXSIZE      128

int32_t devicetree_setup(uint32_t *dt_ptr);

uint32_t devicetree_find_string(char *node, char *prop,
	char *string, uint32_t len);
uint32_t devicetree_find_int(char *node, char *prop, uint32_t *value);
uint64_t devicetree_get_memory(void);
