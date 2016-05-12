@ In assembler because had trouble getting the C compiler
@ to stop doing stupid things

.global interrupt_handler

interrupt_handler:
	sub	lr, lr, #4		@ point LR to actual return address

	ldr	sp,=current_process
	ldr	sp,[sp]

	stmia	sp,{r0-lr}^		@ save all registers and return addr
					@ this is 56 bytes in size

	str	lr,[sp,#60]		@ store saved pc on stack

	mrs	r0, SPSR		@ load SPSR
	str	r0,[sp,#64]		@ store on stack


	add	sp,sp,#8192

	@ Call into the C routine
	bl	interrupt_handler_c

	@ Return from the C routine

exit_interrupt:
	sub	sp,sp,#8192

	ldr	r0,[sp,#64]
	msr	SPSR_cxsf,r0

	ldr	lr,[sp,#60]

	ldmia	sp,{r0-lr}^
	movs	pc,lr			@ return, updating the
					@ CPSR at the same time
