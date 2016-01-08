@ Locks

@ Based on exaples from the "ARM Synchronization Primitives Developmen Article"

.equ	locked,1
.equ	unlocked,0

.global lock_mutex

@ extern void lock_mutex(void *mutex);

lock_mutex:
	ldr	r1,=locked
lock_retry:
	ldrex	r2,[r0]
	cmp	r2,r1
	beq	lock_busy
	strexne	r2,r1,[r0]
	cmpne	r2,#1
	beq	lock_retry

	@ lock was acquired

	@ dmb memory barrier insn not available on ARMv6
	@ use the below instead
	mov	r3,#0
	mcr	p15,0,r3,c7,c10,5

	bx	lr

lock_busy:
@	wait_for_update
	b	lock_retry

@ unlock_mutex
@ extern void unlock_mutex (void *mutex);

.global unlock_mutex

unlock_mutex:
	ldr	r1, =unlocked

	@ dmb memory barrier insn not available on ARMv6
	@ use the below instead
	mov	r3,#0
	mcr	p15,0,r3,c7,c10,5

	str	r1,[r0]
@	signal_udpate
	bx	lr



