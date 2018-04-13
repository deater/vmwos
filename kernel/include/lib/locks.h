#define MUTEX_LOCKED	1
#define MUTEX_UNLOCKED	0

extern void mutex_lock(void *mutex);
extern void mutex_unlock (void *mutex);

