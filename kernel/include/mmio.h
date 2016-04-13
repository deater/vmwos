/* Note on BCM2835 */
/* According to the BCM2835 Peripherals doc we might need memory barriers */
/* Section 1.3 says loads may return out-of-order and unlike the GPU */
/* the ARM chip doesn't have logic to prevent this */
/* It is only strictly needed when switching peripherals */
/* (so a write barrier before first write, a read barrier after last read) */
/* but it's a bit unclear how to make sure that happens in the face of */
/* interrupts. */

/* Linux kernel has a __iowmb(); */
static inline void mmio_write(uint32_t address, uint32_t data) {
	uint32_t *ptr = (uint32_t *)address;
	asm volatile("str %[data], [%[address]]" :
		: [address]"r"(ptr), [data]"r"(data));
}

/* Linux kernel has a __iormb(); */
static inline uint32_t mmio_read(uint32_t address) {
	uint32_t *ptr = (uint32_t *)address;
	uint32_t data;
	asm volatile("ldr %[data], [%[address]]" :
		[data]"=r"(data) : [address]"r"(ptr));
	return data;
}
