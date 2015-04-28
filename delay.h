/* Inline assembly delay loop */

/* volatile means that the compiler should not optimize your code */
/* %= is unique identiier? */

/* assembly */
/*  : output operands */ /* = means write-only, + is read/write r=general reg*/
/*  : input operandss */
/*  : clobbers */

/* clobbers is list of registers that have been changed */
/* memory is possible, as is cc for status flags */

/* can use %[X] to refer to clobbere reg X */
/* that can then use [X]"r"(x) to map to C variable */

static inline void delay(int32_t count) {
        asm volatile("__delay_%=: subs %[count], %[count], #1; "
			"bne __delay_%=\n"
                 : : [count]"r"(count) : "cc");
}

