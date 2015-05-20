

int load_romfs(char *name,char *binary_start,char *stack_start,
		int *size, int *stack_size) {
#if 0
	romfs_open(name);


	/* Allocate Memory */
	binary_start=(char *)memory_allocate(size);
	stack_start=(char *)memory_allocate(stack_size);

	/* Load executable */
	memcpy(binary_start,data,size);
#endif
	return 0;
}
