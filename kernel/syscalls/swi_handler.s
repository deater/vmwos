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
	ldr     sp,=current_process	@ get pointer to current process
        ldr     sp,[sp]			@ and de-reference (sp==current_process)

	stmia	sp,{r0-lr}^		@ Save all user registers r0-lr
					@ (the ^ means user registers)

	str	lr,[sp,#60]		@ store saved PC on stack

	mrs	ip, SPSR		@ load SPSR (assume ip not a swi arg)
	str	ip,[sp,#64]		@ store on stack


	add	sp,sp,#8192		@ our kernel stack is now at the
					@ end of the 8kB process struct

	@ Call the C version of the handler

	bl	swi_handler_c

	@ Put our return value of r0 on the stack so it is
	@ restored with the rest of the saved registers

	sub	sp,sp,#8192		@ restore stack pointer to beginning

	str	r0,[sp]			@ ????

	ldr	r0,[sp,#64]		@ pop saved CPSR
	msr	SPSR_cxsf, r0		@ move it into place

	ldr	lr,[sp,#60]		@ restore address to return to

	@ Restore saved values.  The ^ means to restore the userspace registers
	ldmia	sp, {r0-lr}^
	movs	pc, lr

