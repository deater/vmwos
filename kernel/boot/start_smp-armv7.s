.globl _start_smp

start_smp:
	 /* Get current CPUID */
	/* From the Multiprocessor Affinity Register (MPIDR) */

	/* Note, the desciprtion of MPIDR in the ARMv7 ARM does not match */
	/* well with the actual usage, which is better described in the */
	/* ARM Cortex-A7 manuals */

	mrc	p15, 0, r3, c0, c0, 5
	ands	r3, #3                  /* CPU ID is Bits 0..1 */

	/* set up system stack */
	/* core0 = 0x8000 - 0x7000 */
	/* core1 = 0x7000 - 0x6000 */
	/* core2 = 0x6000 - 0x5000 */
	/* core3 = 0x5000 - 0x4000 */
	/*  so (8-core#)<<12 */

	mov	r0,#8
	sub	r0,r3
	lsl	r0,#12
	mov	sp,r0

	mov	r0,r3			/* put cpu# in arg0 */

	b	secondary_boot_c

