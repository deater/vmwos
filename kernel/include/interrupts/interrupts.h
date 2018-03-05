#include <stdint.h>

/* Get the Current Process Status Register */
/* This includes the flags, as well as things like interrupts being enabled */

static inline uint32_t get_CPSR(void) {

	uint32_t temp;

	asm volatile ("mrs %0,CPSR":"=r" (temp):) ;

	return temp;
}

/* Set the Current Processor Status Register */
/* cxsf = control, extension, status, flags */

static inline void set_CPSR_cxsf(uint32_t new_cpsr) {
	asm volatile ("msr CPSR_cxsf,%0"::"r"(new_cpsr) );
}

/* When setting interrupts, we only really need to set the control flags */
static inline void set_CPSR_c(uint32_t new_cpsr) {
	asm volatile ("msr CPSR_c,%0"::"r"(new_cpsr) );
}

/* enable interrupts  */
/* Enable interrupts by clearing the "Mask Interrupts" flag */
static inline void enable_interrupts(void){
	uint32_t temp;
	temp = get_CPSR();
	set_CPSR_c(temp & ~0x80);
}

/* Disable interrupts */
/* Disable by setting the "Mask Interrupts" flag */
static inline uint32_t disable_interrupts(void){
	uint32_t temp;
	temp = get_CPSR();
	set_CPSR_c(temp | 0x80);
	return temp;
}


int irq_enable(int which);
int irq_disable(int which);
