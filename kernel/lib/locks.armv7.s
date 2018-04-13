.equ	MUTEX_UNLOCKED,	0
.equ	MUTEX_LOCKED,	1


.global mutex_lock

mutex_lock:
	ldr	r1, =MUTEX_LOCKED
lock_retry:
	ldrex	r2, [r0]
	cmp	r2, r1		@ are we already locked?
	wfeeq			@ if so, go to sleep (wait for event)
	beq	lock_retry

	strex	r2, r1, [r0]	@ conditionally store value of r1 into r0
				@ r2 lets you know if it worked or not

	cmp	r2, #1		@ if this failed
	beq	lock_retry	@ then keep retrying

	@ lock was acquired

	dmb			@ Memory barrier
	bx      lr		@ return


.global mutex_unlock		@ in the case of unlock,
				@ there's no need to check the status of lock
mutex_unlock:
	ldr	r1, =MUTEX_UNLOCKED
	dmb			@ memory barrier
	str	r1, [r0]	@ clear the lock
	dmb
	sev			@ wake other processors waiting in wfe
				@ by sending wakeup event

	bx	lr
