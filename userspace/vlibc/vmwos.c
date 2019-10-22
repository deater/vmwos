#include <stdint.h>

#include "vmwos.h"

int vmwos_blink(int value) {

	register long r7 __asm__("r7") = __NR_blink;
	register long r0 __asm__("r0");

	if (value=='n') {
		r0=1;
	}

	if (value=='f') {
		r0=0;
	}

        asm volatile(
                "svc #0\n"
                : "=r"(r0) /* output */
                : "r"(r0), "r"(r7) /* input */
                : "memory");

        return r0;
}

int vmwos_tb1(void) {

	register long r7 __asm__("r7") = __NR_tb1;
	register long r0 __asm__("r0");

	asm volatile(
		"svc #0\n"
		: "=r"(r0) /* output */
		: "r"(r7) /* input */
		: "memory");

	return r0;
}

int vmwos_setfont(int which) {

	register long r7 __asm__("r7") = __NR_setfont;
	register long r0 __asm__("r0") = which;

	asm volatile(
		"svc #0\n"
		: "=r"(r0)
		: "r"(r7), "0"(r0)
		: "memory");

	return r0;
}

int vmwos_gradient(uint32_t type) {

	register long r7 __asm__("r7") = __NR_gradient;
	register long r0 __asm__("r0") = type;

	asm volatile(
		"svc #0\n"
		: "=r"(r0) /* output */
		: "r"(r7), "r"(r0) /* input */
		: "memory");

	return r0;

}

int vmwos_framebuffer_load(int x, int y, int depth, unsigned char *fb) {

	register long r7 __asm__("r7") = __NR_framebuffer_load;
	register long r0 __asm__("r0")=x;
	register long r1 __asm__("r1")=y;
	register long r2 __asm__("r2")=depth;
	register long r3 __asm__("r3")=(unsigned long)fb;

	asm volatile(
		"svc #0\n"
		: "=r"(r0) /* output */
		: "r"(r7), "0"(r0), "r"(r1), "r"(r2), "r"(r3) /* input */
		: "memory");

	return r0;

}


int vmwos_get_temp(void) {

	register long r7 __asm__("r7") = __NR_temp;
	register long r0 __asm__("r0");

	asm volatile(
		"svc #0\n"
		: "=r"(r0)
		: "r"(r7)
		: "memory");

	return r0;
}

int vmwos_random(uint32_t *buffer) {

	register long r7 __asm__("r7") = __NR_random;
	register long r0 __asm__("r0") = (long)buffer;

	asm volatile(
		"svc #0\n"
		: "=r"(r0)
		: "r"(r7), "0"(r0)
		: "memory");

	return r0;
}


void *vmwos_malloc(uint32_t size) {

	register long r7 __asm__("r7") = __NR_malloc;
	register long r0 __asm__("r0") = size;
	register long r1 __asm__("r1") = 1; // memory user

	asm volatile(
		"svc #0\n"
		: "=r"(r0)
		: "r"(r7), "0"(r0), "r"(r1)
		: "memory");

	return (void *)r0;
}

int vmwos_core_poke(uint32_t which) {

	register long r7 __asm__("r7") = __NR_core_poke;
	register long r0 __asm__("r0") = which;

	asm volatile(
		"svc #0\n"
		: "=r"(r0)
		: "r"(r7), "0"(r0)
		: "memory");

	return r0;
}

int vmwos_play_sound(uint32_t *buffer, uint32_t length, uint32_t repeat) {

	register long r7 __asm__("r7") = __NR_play_sound;
	register long r0 __asm__("r0") = (unsigned long)buffer;
	register long r1 __asm__("r1") = length;
	register long r2 __asm__("r2") = repeat;


	asm volatile(
		"svc #0\n"
		: "=r"(r0)
		: "r"(r7), "0"(r0), "r"(r1), "r"(r2)
		: "memory");

	return r0;
}

