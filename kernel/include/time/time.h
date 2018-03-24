struct timespec {
	uint32_t seconds;
	uint32_t ns;
};

/* Must be a power of two */
/* as ARM1176 has no divide instruction */
#define TIMER_HZ 64

extern uint32_t tick_counter;

/* This will overflow in roughly 2 years */
uint32_t time_since_boot(void);

uint32_t ticks_since_boot(void);

int32_t clock_gettime(uint32_t clk_id,struct timespec *t);
