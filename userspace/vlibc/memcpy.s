@ Copyright (C) 2006-2015 Free Software Foundation, Inc.
@   This file is part of the GNU C Library.
@   Contributed by MontaVista Software, Inc. (written by Nicolas Pitre)
@   The GNU C Library is free software; you can redistribute it and/or
@   modify it under the terms of the GNU Lesser General Public
@   License as published by the Free Software Foundation; either
@   version 2.1 of the License, or (at your option) any later version.
@   The GNU C Library is distributed in the hope that it will be useful,
@   but WITHOUT ANY WARRANTY; without even the implied warranty of
@   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
@   Lesser General Public License for more details.
@   You should have received a copy of the GNU Lesser General Public
@   License along with the GNU C Library.  If not, see
@   <http://www.gnu.org/licenses/>.  */



.if 0
# define cfi_offset(reg, off)                .cfi_offset reg, off
# define cfi_rel_offset(reg, off)        .cfi_rel_offset reg, off
# define cfi_register(r1, r2)                .cfi_register r1, r2
# define cfi_return_column(reg)        .cfi_return_column reg
# define cfi_restore(reg)                .cfi_restore reg
# define cfi_same_value(reg)                .cfi_same_value reg
# define cfi_undefined(reg)                .cfi_undefined reg
# define cfi_remember_state                .cfi_remember_state
# define cfi_restore_state                .cfi_restore_state
# define cfi_window_save                .cfi_window_save
# define cfi_personality(enc, exp)        .cfi_personality enc, exp
# define cfi_lsda(enc, exp)                .cfi_lsda enc, exp
.endif

.equ ARM_BX_ALIGN_LOG2, 2

/* Prototype: void *memcpy(void *dest, const void *src, size_t n); */
.globl	memcpy
memcpy:
		push	{r0, r4, lr}
		subs	r2, r2, #4
		blt	8f
		ands	ip, r0, #3
		pld	[r1, #0]
		bne	9f
		ands	ip, r1, #3
		bne	10f
1:		subs	r2, r2, #(28)
		push	{r5 - r8}
		blt	5f

		pld	[r1, #0]
2:		subs	r2, r2, #96
		pld	[r1, #28]
		blt	4f
		pld	[r1, #60]
		pld	[r1, #92]
3:		pld	[r1, #124]
4:		ldmia	r1!, {r3, r4, r5, r6, r7, r8, ip, lr}
		subs	r2, r2, #32
		stmia	r0!, {r3, r4, r5, r6, r7, r8, ip, lr}
		bge	3b
		cmn	r2, #96
		bge	4b
5:		ands	ip, r2, #28
		rsb	ip, ip, #32

		/* C is always clear here.  */
		addne	pc, pc, ip, lsl #(ARM_BX_ALIGN_LOG2 - 2)
		b	7f

		.p2align ARM_BX_ALIGN_LOG2
6:		nop
		.p2align ARM_BX_ALIGN_LOG2
		ldr	r3, [r1], #4
		.p2align ARM_BX_ALIGN_LOG2
		ldr	r4, [r1], #4
		.p2align ARM_BX_ALIGN_LOG2
		ldr	r5, [r1], #4
		.p2align ARM_BX_ALIGN_LOG2
		ldr	r6, [r1], #4
		.p2align ARM_BX_ALIGN_LOG2
		ldr	r7, [r1], #4
		.p2align ARM_BX_ALIGN_LOG2
		ldr	r8, [r1], #4
		.p2align ARM_BX_ALIGN_LOG2
		ldr	lr, [r1], #4

		add	pc, pc, ip, lsl #(ARM_BX_ALIGN_LOG2 - 2)
		nop

		.p2align ARM_BX_ALIGN_LOG2
66:		nop
		.p2align ARM_BX_ALIGN_LOG2
		str	r3, [r0], #4
		.p2align ARM_BX_ALIGN_LOG2
		str	r4, [r0], #4
		.p2align ARM_BX_ALIGN_LOG2
		str	r5, [r0], #4
		.p2align ARM_BX_ALIGN_LOG2
		str	r6, [r0], #4
		.p2align ARM_BX_ALIGN_LOG2
		str	r7, [r0], #4
		.p2align ARM_BX_ALIGN_LOG2
		str	r8, [r0], #4
		.p2align ARM_BX_ALIGN_LOG2
		str	lr, [r0], #4

7:		pop	{r5 - r8}
8:		movs	r2, r2, lsl #31
		ldrneb	r3, [r1], #1
		ldrcsb	r4, [r1], #1
		ldrcsb	ip, [r1]
		strneb	r3, [r0], #1
		strcsb	r4, [r0], #1
		strcsb	ip, [r0]

		pop	{r0, r4, pc}

9:		rsb	ip, ip, #4
		cmp	ip, #2
		ldrgtb	r3, [r1], #1
		ldrgeb	r4, [r1], #1
		ldrb	lr, [r1], #1
		strgtb	r3, [r0], #1
		strgeb	r4, [r0], #1
		subs	r2, r2, ip
		strb	lr, [r0], #1
		blt	8b
		ands	ip, r1, #3
		beq	1b
10:		bic	r1, r1, #3
		cmp	ip, #2
		ldr	lr, [r1], #4
		beq	17f
		bgt	18f
		.macro	forward_copy_shift pull push
		subs	r2, r2, #28
		blt	14f
11:		push	{r5 - r8, r10}
		pld	[r1, #0]
		subs	r2, r2, #96
		pld	[r1, #28]
		blt	13f
		pld	[r1, #60]
		pld	[r1, #92]
12:		pld	[r1, #124]
13:
		ldmia	r1!, {r4, r5, r6, r7}
		mov	r3, lr, lsr #\pull
		subs	r2, r2, #32
		ldmia	r1!, {r8, r10, ip, lr}
		orr	r3, r3, r4, lsl #\push
		mov	r4, r4, lsr #\pull
		orr	r4, r4, r5, lsl #\push
		mov	r5, r5, lsr #\pull
		orr	r5, r5, r6, lsl #\push
		mov	r6, r6, lsr #\pull
		orr	r6, r6, r7, lsl #\push
		mov	r7, r7, lsr #\pull
		orr	r7, r7, r8, lsl #\push
		mov	r8, r8, lsr #\pull
		orr	r8, r8, r10, lsl #\push
		mov	r10, r10, lsr #\pull
		orr	r10, r10, ip, lsl #\push
		mov	ip, ip, lsr #\pull
		orr	ip, ip, lr, lsl #\push
		stmia	r0!, {r3, r4, r5, r6, r7, r8, r10, ip}
		bge	12b
		cmn	r2, #96
		bge	13b
		pop	{r5 - r8, r10}
14:		ands	ip, r2, #28
		beq	16f
15:		mov	r3, lr, lsr #\pull
		ldr	lr, [r1], #4
		subs	ip, ip, #4
		orr	r3, r3, lr, lsl #\push
		str	r3, [r0], #4
		bgt	15b
16:		sub	r1, r1, #(\push / 8)
		b	8b
		.endm
		forward_copy_shift	pull=8	push=24
17:		forward_copy_shift	pull=16	push=16
18:		forward_copy_shift	pull=24	push=8


