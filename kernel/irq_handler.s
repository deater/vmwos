@ In assembler because had trouble getting the C compiler
@ to stop doing stupid things

.global interrupt_handler

interrupt_handler:
	sub	lr, lr, #4		@ point LR to actual return address

	sub	sp,sp,#56
	stmia	sp,{r0-lr}^		@ save all registers and return addr
					@ this is 56 bytes in size

	push	{lr}			@ store saved pc on stack

	mrs	r0, SPSR		@ load SPSR
	push	{r0}			@ store on stack


	@ Call into the C routine
	mov	r0,sp
	bl	interrupt_handler_c

	@ Return from the C routine

exit_interrupt:
	pop	{r0}
	msr	SPSR_cxsf,r0

	pop	{lr}

	ldmia	sp,{r0-lr}^
	add	sp,sp,#56
	movs	pc,lr			@ return, updating the
					@ CPSR at the same time
