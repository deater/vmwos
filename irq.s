
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

	bl	interrupt_handle_timer

	mov	r0,sp

	bl	schedule

	add	sp, sp, #72
	ldm     sp!, {r0 - r12, pc}^

