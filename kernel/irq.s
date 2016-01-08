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

	mov	r1,#0			@ handled

	@ Check if GPIO23 (ps2 keyboard) (irq49)
check_gpio:
	ldr	r0,=0x2000b208		@ IRQ_PENDING2
	ldr	r0,[r0]
	tst	r0,#0x20000		@ bit 17 (irq49)
	beq	check_uart		@ not set, skip to uart

	add	r1,r1,#1		@ handled
	push	{r1}

	bl	ps2_interrupt_handler
	pop	{r1}

	@ Check if UART (irq57)
check_uart:

	ldr	r0,=0x2000b200		@ IRQ_BASIC_PENDING
	ldr	r0,[r0]
	tst	r0,#(1<<19)
	beq	check_timer		@ not set, skip to timer

	add	r1,r1,#1
	push	{r1}
	bl	uart_interrupt_handler
	pop	{r1}

check_timer:
	@ check if it's a timer interrupt
	@ It's in the basic pending register, bit 0

	ldr	r0,=0x2000b200		@ IRQ_BASIC_PENDING
	ldr	r0,[r0]
	tst	r0,#0x1
	bne	timer_interrupt

unknown_interrupt:

	cmp	r1,#0

	@ Unknown interrupt
	bleq	interrupt_handle_unknown
	b	exit_interrupt

timer_interrupt:
	bl	interrupt_handle_timer

	mov	r0,sp

	bl	schedule

exit_interrupt:
	add	sp, sp, #72
	ldm     sp!, {r0 - r12, pc}^

