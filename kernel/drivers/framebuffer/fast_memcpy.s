@ VFP2 copy code idea comes from looking at PiFox by Team28

@ turns out this is not as fast as the linux/glibc memcpy

/* fast_fb_update((char *)current_fb.pointer,pointer); */

.globl	fast_fb_update
fast_fb_update:
		@ dest is in r0
		@ src is in r1

		stmfd	sp!, {r0 - r3}		@ save on stack

		@ want to copy 640*480*4 bytes
		mov	r2,#(640*480*4)

		@ copy 64 bytes at a time
fast_fb_loop:
		vldm	r1!, {d0-d7}		@ load 64 bytes from source
		vstm	r0!, {d0-d7}		@ store 64 bytes to dest
		subs	r2,r2,#0x40		@ subtract 64 from count
		bne	fast_fb_loop		@ go until done

		ldmfd	sp!, {r0 - r3}		@ restore saved vars
		mov	pc, lr			@ return

