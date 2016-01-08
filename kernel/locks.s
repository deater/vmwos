@ Locks

@ Based on exaples from the "ARM Synchronization Primitives Developmen Article"

.equ	locked,1
.equ	unlocked,0

.global lock_mutex

@ extern void lock_mutex(void *mutex);

lock_mutex:
	ldr	r1,=locked	@ Load 1 into r1
lock_retry:
	ldrex	r2,[r0]		@ load/mark exclusive from mutex
	cmp	r2,r1		@ is it locked?
	beq	lock_busy	@ if so, busy
	strexne	r2,r1,[r0]	@ conditionally store the value in r1 to r0
				@ (i.e. "locked" to the mutex)
				@ r2 indicates whether this worked or not
	cmpne	r2,#1		@ check to see if it worked
	beq	lock_retry	@ if it failed, then retry

	@ lock was acquired

	@ dmb memory barrier insn not available on ARMv6
	@ use the below instead
	mov	r3,#0
	mcr	p15,0,r3,c7,c10,5

	bx	lr		@ return

lock_busy:
				@ Ideally we would do something low-power
				@ besides just busy-waiting here
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

	str	r1,[r0]			@ clear the lock
@	signal_udpate			@ more advanced systems
					@ we would wake sleepers here
	bx	lr



