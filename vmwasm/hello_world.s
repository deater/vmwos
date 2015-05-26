.arm

@ Syscall Definitions
.equ SYSCALL_EXIT,     1
.equ SYSCALL_WRITE,    4

@ Other Definitions
.equ STDOUT,	       1

        .globl _start
_start:

	mov	r0,#0

	ldr	r1,=message
	bl	print_string

        @================================
        @ Exit
        @================================
exit:
	mov	r0,#0			@ Return a zero
        mov	r7,#SYSCALL_EXIT	@
        swi	0x0			@ Run the system call


	@====================
	@ print_string
	@====================
	@ Null-terminated string to print pointed to by r1
	@ the value in r1 is destroyed by this routine


print_string:

	push    {r0,r2,r7,r10,lr}	// Save r0,r2,r7,r10,lr on stack

	mov	r2,#0

count_loop:
	ldrb	r0,[r1,r2]
	add	r2,r2,#1
	bne	count_loop

	@ The length of the string pointed to by r1
	@ Should be put in r2 by your code above

	mov	r0,#STDOUT		// R0 Print to stdout
					// R1 points to our string
					// R2 is the length
	mov	r7,#SYSCALL_WRITE	// Load syscall number
	swi	0x0			// System call

	pop	{r0,r2,r7,r10,pc}       // pop r0,r2,r3,pc from stack

.data
message:	.string "Hello Wolrd!\r\n"

@ BSS
.lcomm buffer,11
