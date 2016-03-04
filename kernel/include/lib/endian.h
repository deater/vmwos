static inline uint32_t htonl(uint32_t x) {
	__asm__("rev %0, %0" : "+r"(x));
	return x;
}

static inline uint32_t ntohl(uint32_t x) {
	__asm__("rev %0, %0" : "+r"(x));
	return x;
}
