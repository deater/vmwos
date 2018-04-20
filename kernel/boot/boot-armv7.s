/* Minimal assembly language startup code */
/* This does just enough to prepare a C   */
/* Program for execution on Raspberry-Pi  */

/* Pi2/Pi3 code based on code from */
/* https://github.com/vanvught/rpidmx512/firmware-template/vectors.s */
/* Which is based on the Linux kernel code */

/* Definitions for Mode bits and Interrupt Flags */

.equ	CPSR_MODE_USER,		0x10
.equ	CPSR_MODE_FIQ,		0x11
.equ	CPSR_MODE_IRQ,		0x12
.equ	CPSR_MODE_SVC,		0x13
.equ	CPSR_MODE_ABORT,	0x17
.equ	CPSR_MODE_UNDEFINED,	0x1b
.equ	CPSR_MODE_SYSTEM,	0x1f

.equ	CPSR_MODE_FIQ_DISABLE,	(1<<6)		@ F set, FIQ disabled
.equ	CPSR_MODE_IRQ_DISABLE,	(1<<7)		@ I set, IRQ disabled
.equ	CPSR_MODE_ABORT_DISABLE,(1<<8)		@ A set, ABT disabled

/* Put this in the boot section, this matches */
/* The declaration in the linker script       */
.section ".text.boot"

.code	32
.align	2

/* Make _start globally visible so that the linker can see it */
.globl _start

/* The bootloader starts, loads are executable, and enters */
/* execution at 0x8000 with the following values set.      */
/* r0 = boot method (usually 0 on pi)       		   */
/* r1 = hardware type (usually 0xc42 on pi) 		   */
/* r2 = points to device tree file			   */
/*	on older firmware points to ATAGS (usually 0x100)  */

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

/* Pi2/Pi3 only!!! */
	/* Get current CPUID */
	/* From the Multiprocessor Affinity Register (MPIDR) */

	/* Note, the desciprtion of MPIDR in the ARMv7 ARM does not match */
	/* well with the actual usage, which is better described in the */
	/* ARM Cortex-A7 manuals */

	mrc	p15, 0, r3, c0, c0, 5
	ands	r3, #3			/* CPU ID is Bits 0..1 */
	bne	wait_forever		/* If not CPU zero, go to sleep */
					/* In theory recent firmware already */
					/* Did this for us */

	cpsid	if			/* Disable IRQ/FIR interrupts */
					/* The cpsid instruction is a */
					/* shortcut to setting those  */
					/* bits in the CPSR */

	/* Check for hypervisor mode */
	/* As the firmware stub will put pi2/pi3 into HYP mode */

	mrs	r3, cpsr		/* put cpsr in r3 */
	and	r4, r3, #0x1F		/* mask off all but mode */
	cmp	r4, #0x1A		/* check for HYP (0x1A) */
	bne	done_hyp		/* skip ahead if not in HYP mode */

	/* If we're here, we are in HYP mode */
	bic	r3, r3, #0x1F		/* clear the mode bits */
					/* Setup SVC mode, IRQs disabled */
	orr	r3, #CPSR_MODE_SVC | CPSR_MODE_IRQ_DISABLE | CPSR_MODE_FIQ_DISABLE
					/* mask Abort bit */
					/* Separate as it's an invalid constant */
					/* If we try to include with above */
	orr	r3, #CPSR_MODE_ABORT_DISABLE

	adr	lr, done_pi2		/* load address we want to return to */
	msr	spsr_cxsf, r3		/* update spsr, cxsf = everything */
	msr	ELR_hyp, lr		/* Special return address when in */
					/* hypervisor mode */
	eret				/* Special return-from hypervisor */
					/* instruction */
done_hyp:
					/* Make sure we are in SVC mode */
	orr	r3, r3, #CPSR_MODE_SVC | CPSR_MODE_IRQ_DISABLE | CPSR_MODE_FIQ_DISABLE
	msr	cpsr_c, r3		/* set the control (mode) bits */
done_pi2:

/* End Pi2/Pi3 only!!! */

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

	/* Setup the other stacks.  Share a stack		*/
	/* is that an issue?  hopefully these are unlikely	*/

	/* Set up the Undefined Mode Stack	*/
	mov	r3, #(CPSR_MODE_UNDEFINED | CPSR_MODE_IRQ_DISABLE | CPSR_MODE_FIQ_DISABLE )
	msr	cpsr_c, r3
	mov	sp, #0x2000
	/* Switch back to supervisor mode */
	mov	r3, #(CPSR_MODE_SVC | CPSR_MODE_IRQ_DISABLE | CPSR_MODE_FIQ_DISABLE )
	msr	cpsr_c, r3

	/* Set up the Abort Mode Stack	*/
	/* First switch to interrupt mode, then update stack pointer */
	mov	r3, #(CPSR_MODE_ABORT | CPSR_MODE_IRQ_DISABLE | CPSR_MODE_FIQ_DISABLE )
	msr	cpsr_c, r3
	mov	sp, #0x2000
	/* Switch back to supervisor mode */
	mov	r3, #(CPSR_MODE_SVC | CPSR_MODE_IRQ_DISABLE | CPSR_MODE_FIQ_DISABLE )
	msr	cpsr_c, r3

	/* Set up the FIQ Mode Stack	*/
	mov	r3, #(CPSR_MODE_FIQ | CPSR_MODE_IRQ_DISABLE | CPSR_MODE_FIQ_DISABLE )
	msr	cpsr_c, r3
	mov	sp, #0x3000
	/* Switch back to supervisor mode */
	mov	r3, #(CPSR_MODE_SVC | CPSR_MODE_IRQ_DISABLE | CPSR_MODE_FIQ_DISABLE )
	msr	cpsr_c, r3



	/* copy irq vector into place.  Preserve r0,r1,r2 */
	/* Note: irq vector defaults to 0x000000 but this is */
	/*       configurable on the Pi2/Pi3                 */
	/* TODO: should we make sure the various bits are set right? */

	ldr	r3, =_start
	mov	r4, #0x0000
	/* Quick way to copy 256 bytes of memory */
	ldmia	r3!,{r5, r6, r7, r8, r9, r10, r11, r12}
	stmia	r4!,{r5, r6, r7, r8, r9, r10, r11, r12}
	ldmia	r3!,{r5, r6, r7, r8, r9, r10, r11, r12}
	stmia	r4!,{r5, r6, r7, r8, r9, r10, r11, r12}


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
