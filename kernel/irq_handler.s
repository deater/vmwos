@ In assembler because had trouble getting the C compiler
@ to stop doing stupid things

.global interrupt_handler

interrupt_handler:
	sub	lr, lr, #4		@ point LR to actual return address
	push    {r0 - r12 , lr}		@ save all registers and return addr

	sub	sp,sp,#72		@ point stack down 13 words lower
	stm	sp,{r0 - lr}^		@ save current user context there
					@ the ^ means user registers
	mrs	r0, SPSR		@ load SPSR
	str	r0, [sp,#60]		@ store at offset 60
	str	lr, [sp,#64]		@ save LR at offser 64
	mov	r0,sp			@ get stack pointer
	add	r0,r0,#(72+56)		@ FIXME: what?
	str	r0, [sp,#68]

	bl	interrupt_handler_c

exit_interrupt:
	add	sp, sp, #72		@ restore stack
	ldm     sp!, {r0 - r12, pc}^	@ return, updating the
					@ CPSR at the same time
