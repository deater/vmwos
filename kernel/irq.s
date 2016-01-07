@ In assembler because had trouble getting the C compiler
@ to stop doing stupid things

.global interrupt_handler

interrupt_handler:
	sub	lr, lr, #4
	push    {r0 - r12 , lr}

	sub	sp,sp,#72
	stm	sp,{r0 - lr}^
	mrs	r0, SPSR
	str	r0, [sp,#60]
	str	lr, [sp,#64]
	mov	r0,sp
	add	r0,r0,#(72+56)
	str	r0, [sp,#68]

	@ check if it's a timer interrupt
	@ It's in the basic pending register, bit 0

	ldr	r0,=0x2000b200
	ldr	r0,[r0]
	tst	r0,#0x1
	bne	timer_interrupt

	@ Unknown interrupt
	bl	interrupt_handle_unknown
	b	exit_interrupt

timer_interrupt:
	bl	interrupt_handle_timer

	mov	r0,sp

	bl	schedule

exit_interrupt:
	add	sp, sp, #72
	ldm     sp!, {r0 - r12, pc}^

