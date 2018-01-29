uint32_t mini_uart_init(struct serial_type *serial);
void mini_uart_putc(unsigned char byte);
int32_t mini_uart_getc(void);
int32_t mini_uart_getc_noblock(void);
void mini_uart_enable_interrupts(void);
int32_t mini_uart_interrupt_handler(void);
