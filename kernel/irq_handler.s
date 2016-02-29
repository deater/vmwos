@ In assembler because had trouble getting the C compiler
@ to stop doing stupid things

.global interrupt_handler

interrupt_handler:
	sub	lr, lr, #4		@ point LR to actual return address
	push    {r0 - r12 , lr}		@ save all registers and return addr
					@ this is 56 bytes in size

	sub	sp,sp,#72		@ point stack down 18 words lower
	stm	sp,{r0 - lr}^		@ save current user context there
					@ the ^ means user registers
					@ (0 - 56)
	mrs	r0, SPSR		@ load SPSR
	str	r0, [sp,#60]		@ store at offset 60
	str	lr, [sp,#64]		@ save LR (PC at time of interrupt)
					@ to offset 64

	mov	r0,sp			@ get pointer to original SP at entry
	add	r0,r0,#(72+56)		@ hack to point back to before first
	str	r0, [sp,#68]		@ push, store at offset 68
					@ the scheduler uses this to fix
					@ stack if we context switch

	@ Call into the C routine

	bl	interrupt_handler_c

	@ Return from the C routine

exit_interrupt:
	add	sp, sp, #72		@ restore stack
	ldm     sp!, {r0 - r12, pc}^	@ return, updating the
					@ CPSR at the same time
