.global swi_handler

	@ Wrapper for the SWI handler
	@ You can do this in gcc with __attribute__((interrupt("SWI")))
	@ but it doesn't handle returning the value in r0

	@ TODO: save SPSR, etc?

	@ Note: currently do not call swi from SVC mode or things
	@       will get corrupted


	@ Note, because it's a pain to find, the Linux equivelent is in
	@	entry-armv.S and entry-common.S

swi_handler:

	sub	sp,sp,#56
	stmia	sp,{r0-lr}^		@ Save all user registers r0-lr

	push	{lr}			@ store saved PC on stack
					@ (the ^ means user registers)

	mrs	ip, SPSR		@ load SPSR (assume ip not a swi arg)
	push	{ip}			@ store on stack

	ldr	ip,=swi_handler_stack
	str	sp,[ip]

	@ Call the C version of the handler

	bl	swi_handler_c

	@ Put our return value of r0 on the stack so it is
	@ restored with the rest of the saved registers

	str	r0,[sp,#8]

	pop	{r0}			@ pop saved CPSR
	msr	SPSR_cxsf, r0		@ move it into place

	pop	{lr}			@ restore address to return to

	@ Restore saved values.  The ^ means to restore the userspace registers
	ldmia	sp, {r0-lr}^
	add	sp, sp, #56
	movs	pc, lr

