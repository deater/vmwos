/* Minimal assembly language startup code */
/* Sets up a Raspberry Pi-1 for launching into C */

/* Definitions for Mode bits and Interrupt Flags */

.equ    CPSR_MODE_USER,         0x10
.equ    CPSR_MODE_FIQ,          0x11
.equ    CPSR_MODE_IRQ,          0x12
.equ    CPSR_MODE_SVC,          0x13
.equ    CPSR_MODE_ABORT,        0x17
.equ    CPSR_MODE_UNDEFINED,    0x1b
.equ    CPSR_MODE_SYSTEM,       0x1f

.equ    CPSR_MODE_FIQ_DISABLE,  (1<<6)          @ F set, FIQ disabled
.equ    CPSR_MODE_IRQ_DISABLE,  (1<<7)          @ I set, IRQ disabled
.equ    CPSR_MODE_ABORT_DISABLE,(1<<8)          @ A set, ABT disabled


/* Put this in the boot section, this matches */
/* The declaration in the linker script       */
.section ".text.boot"

/* Make _start globally visible so that the linker can see it */
.globl _start

/* The bootloader starts, loads are executable, and enters */
/* execution at 0x8000 with the following values set.      */
/* r0 = boot method (usually 0 on pi)       		   */
/* r1 = hardware type (usually 0xc42 on pi) 		   */
/* r2 = start of ATAGS ARM tag boot info (usually 0x100)   */

_start:

	/* Setup the interrupt vectors */
	/* This code gets copied to address 0x0000 (irq vector table)	*/
	/* The next instructions jumps us to the "reset" vector		*/
	/* Where we continued our boot code.				*/
	ldr	pc, reset_addr
	ldr	pc, undefined_instruction_addr
	ldr	pc, software_interrupt_addr
	ldr	pc, prefetch_abort_addr
	ldr	pc, data_abort_addr
	ldr	pc, unused_handler_addr
	ldr	pc, interrupt_addr
	ldr	pc, fast_interrupt_addr
reset_addr:			.word	reset
undefined_instruction_addr:	.word	undef_handler
software_interrupt_addr:	.word	swi_handler
prefetch_abort_addr:		.word	prefetch_abort_handler
data_abort_addr:		.word	data_abort_handler
unused_handler_addr:		.word	undef_handler
interrupt_addr:			.word	interrupt_handler
fast_interrupt_addr:		.word	fiq_handler
	/* Done Interrupt vector block */

	/* Continue with boot code */
reset:

	/* Set up the Supervisor Mode Stack	*/
	/* Put it right before the entry point	*/
	/* (it grows down)			*/
	mov	sp, #0x8000

	/* Set up the Interrupt Mode Stack	*/
	/* First switch to interrupt mode, then update stack pointer */
	mov	r3, #(CPSR_MODE_IRQ | CPSR_MODE_IRQ_DISABLE | CPSR_MODE_FIQ_DISABLE )
	msr	cpsr_c, r3
	mov	sp, #0x4000
	/* Switch back to supervisor mode */
	mov	r3, #(CPSR_MODE_SVC | CPSR_MODE_IRQ_DISABLE | CPSR_MODE_FIQ_DISABLE )
	msr	cpsr_c, r3


	/* Setup the other stacks.  Share a stack               */
	/* is that an issue?  hopefully these are unlikely      */

	/* Set up the Undefined Mode Stack      */
	mov	r3, #(CPSR_MODE_UNDEFINED | CPSR_MODE_IRQ_DISABLE | CPSR_MODE_FIQ_DISABLE )
	msr	cpsr_c, r3
	mov	sp, #0x2000
        mov	r3, #(CPSR_MODE_SVC | CPSR_MODE_IRQ_DISABLE | CPSR_MODE_FIQ_DISABLE )
	msr	cpsr_c, r3

        /* Set up the Abort Mode Stack  */
	mov	r3, #(CPSR_MODE_ABORT | CPSR_MODE_IRQ_DISABLE | CPSR_MODE_FIQ_DISABLE )
	msr	cpsr_c, r3
	mov	sp, #0x2000
	mov	r3, #(CPSR_MODE_SVC | CPSR_MODE_IRQ_DISABLE | CPSR_MODE_FIQ_DISABLE )
	msr	cpsr_c, r3


	/* copy irq vector2 into place.  Preserve r0,r1,r2 */
	ldr	r3, =_start
	mov     r4, #0x0000
	/* Quick way to copy 256 bytes of memory */
	ldmia   r3!,{r5, r6, r7, r8, r9, r10, r11, r12}
	stmia   r4!,{r5, r6, r7, r8, r9, r10, r11, r12}
	ldmia   r3!,{r5, r6, r7, r8, r9, r10, r11, r12}
	stmia   r4!,{r5, r6, r7, r8, r9, r10, r11, r12}


	/* clear the bss section */
	/* This has not been optimized */
	ldr	r4, =__bss_start
	ldr	r9, =__bss_end
	mov	r5, #0
clear_bss:
	str	r5,[r4]
	add	r4,r4,#4

	cmp	r4,r9
	ble 	clear_bss

	/* Size of the kernel? */
	ldr	r3, =__bss_end

	/* Call our main function */
	/* The values r0 - r2 from bootloader are preserved */
	ldr r4, =kernel_main
	blx r4

	/* We only reach this if the code we call exits */
wait_forever:
	wfe	/* wait for event -- put CPU to sleep */
	b wait_forever
