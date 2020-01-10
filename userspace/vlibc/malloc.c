#include <stdint.h>
#include <stddef.h>

#include "string.h"
#include "vmwos.h"


struct block_meta {
	size_t size;
	struct block_meta *next;
	int free;
	int magic;
};

#define META_SIZE sizeof(struct block_meta)

static void *global_base = NULL;

static struct block_meta *find_free_block(
				struct block_meta **last,size_t size) {

	struct block_meta *current = global_base;

	while (current && !(current->free && current->size >= size)) {
		*last = current;
		current = current->next;
	}

	return current;
}

struct block_meta *request_space(struct block_meta* last, size_t size) {

	struct block_meta *block;

	void *request = vmwos_malloc(size + META_SIZE);
	block=request;

	//assert((void*)block == request); // Not thread safe.
	if (request == (void*) -1) {
		return NULL; // sbrk failed.
	}

	if (last) { // NULL on first request.
		last->next = block;
	}
	block->size = size;
	block->next = NULL;
	block->free = 0;
	block->magic = 0x12345678;

	return block;
}

	/* Helper function */
struct block_meta *get_block_ptr(void *ptr) {
	return (struct block_meta*)ptr - 1;
}

void *malloc(uint32_t size) {

	struct block_meta *block;

	// TODO: align size?

	if (size <= 0) {
		return NULL;
	}

	if (!global_base) { // First call.
		block = request_space(NULL, size);
		if (!block) {
			return NULL;
		}
		global_base = block;
	} else {
		struct block_meta *last = global_base;

		block = find_free_block(&last, size);
		if (!block) { // Failed to find free block.
			block = request_space(last, size);
			if (!block) {
				return NULL;
			}
		} else {      // Found free block
			// TODO: consider splitting block here.
			block->free = 0;
			block->magic = 0x77777777;
		}
	}

	return(block+1);

}

void free(void *ptr) {

	if (!ptr) {
		return;
	}

	// TODO: consider merging blocks once splitting blocks is implemented.
	struct block_meta* block_ptr = get_block_ptr(ptr);

	//assert(block_ptr->free == 0);
	//assert(block_ptr->magic == 0x77777777 || block_ptr->magic == 0x12345678);
	block_ptr->free = 1;
	block_ptr->magic = 0x55555555;

	return;
}

void *calloc(uint32_t nmemb, uint32_t size) {

	size_t total_size = nmemb * size; // TODO: check for overflow.
	void *ptr = malloc(total_size);
	memset(ptr, 0, total_size);
	return ptr;
}

void *realloc(void *ptr, uint32_t size) {

	if (!ptr) {
		// NULL ptr. realloc should act like malloc.
		return malloc(size);
	}

	struct block_meta* block_ptr = get_block_ptr(ptr);
	if (block_ptr->size >= size) {
		// We have enough space. Could free some once we implement split.
		return ptr;
	}

	// Need to really realloc. Malloc new space and free old space.
	// Then copy old data to new space.
	void *new_ptr;
	new_ptr = malloc(size);
	if (!new_ptr) {
		return NULL; // TODO: set errno on failure.
	}
	memcpy(new_ptr, ptr, block_ptr->size);
	free(ptr);
	return new_ptr;
}

char *strdup(const char *s) {

	int64_t string_size;
	char *new_string=NULL;

	string_size=strlen(s);

	new_string=malloc(string_size+1);
	if (new_string!=NULL) {
		memcpy(new_string,s,string_size);
		new_string[string_size]='\0';
	}

	return new_string;
}
