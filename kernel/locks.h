#define MUTEX_LOCKED	1
#define MUTEX_UNLOCKED	0

extern void lock_mutex(void *mutex);
extern void unlock_mutex (void *mutex);

