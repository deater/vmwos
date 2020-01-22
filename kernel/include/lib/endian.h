#ifdef VMWOS

static inline uint32_t htonl(uint32_t x) {
	__asm__("rev %0, %0" : "+r"(x));
	return x;
}

static inline uint32_t ntohl(uint32_t x) {
	__asm__("rev %0, %0" : "+r"(x));
	return x;
}

#else

#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define htonl(x) (x)
#define ntohl(x) (x)
#else
#define htonl(x) ( ((x)<<24 & 0xFF000000UL) | \
                   ((x)<< 8 & 0x00FF0000UL) | \
                   ((x)>> 8 & 0x0000FF00UL) | \
                   ((x)>>24 & 0x000000FFUL) )
#define ntohl(x) ( ((x)<<24 & 0xFF000000UL) | \
                   ((x)<< 8 & 0x00FF0000UL) | \
                   ((x)>> 8 & 0x0000FF00UL) | \
                   ((x)>>24 & 0x000000FFUL) )
#endif

#endif
