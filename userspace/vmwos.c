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

int vmwos_gradient(void) {

	register long r7 __asm__("r7") = __NR_gradient;
	register long r0 __asm__("r0");

	asm volatile(
		"svc #0\n"
		: "=r"(r0) /* output */
		: "r"(r7) /* input */
		: "memory");

	return r0;

}

int vmwos_run(int which) {

	register long r7 __asm__("r7") = __NR_run;
	register long r0 __asm__("r0") = which;

	asm volatile(
		"svc #0\n"
		: "=r"(r0)
		: "r"(r7), "0"(r0)
		: "memory");

	return r0;
}

int vmwos_stop(int which) {

	register long r7 __asm__("r7") = __NR_stop;
	register long r0 __asm__("r0") = which;

	asm volatile(
		"svc #0\n"
		: "=r"(r0)
		: "r"(r7), "0"(r0)
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
