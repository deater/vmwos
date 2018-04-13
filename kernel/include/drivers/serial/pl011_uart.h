void pl011_uart_enable_locking(void);
uint32_t pl011_uart_init(struct serial_type *serial);
void pl011_uart_putc(unsigned char byte);
int32_t pl011_uart_getc(void);
int32_t pl011_uart_getc_noblock(void);
void pl011_uart_enable_interrupts(void);
int32_t pl011_uart_interrupt_handler(void);
