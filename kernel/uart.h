void uart_init(void);
void uart_putc(unsigned char byte);
uint32_t uart_getc(void);
uint32_t uart_getc_noblock(void);
uint32_t uart_write(const unsigned char *buffer, size_t size);
void uart_enable_interrupts(void);
