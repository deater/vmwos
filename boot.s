/* Minimal assembly language startup code */
/* This does just enough to prepare a C   */
/* Program for execution on Raspberry-Pi  */

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
	/* Set up the stack to be right before our entry point	*/
	/* (it grows down)					*/
	mov	sp, #0x8000

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
 
	/* Call our main function */
	/* The values r0 - r2 from bootloader are preserved */
	ldr r3, =main
	blx r3

	/* We only reach this if the code we call exits */
wait_forever:
	wfe	/* wait for event -- put CPU to sleep */
	b wait_forever
