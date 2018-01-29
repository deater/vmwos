#define SERIAL_UART_PL011	1
#define SERIAL_UART_MINI	2

#define SERIAL_PARITY_NONE	0
#define SERIAL_PARITY_ODD	1
#define SERIAL_PARITY_EVEN	2


struct serial_type {
        uint32_t initialized;
        uint32_t baud;
        uint32_t bits;
        uint32_t stop;
        uint32_t parity;
        void (*uart_enable_interrupts)(void);
        void (*uart_putc)(unsigned char byte);
        int32_t (*uart_getc)(void);
        int32_t (*uart_getc_noblock)(void);
        int32_t (*uart_interrupt_handler)(void);
};

uint32_t serial_init(uint32_t type);
uint32_t serial_write(const char *buffer, size_t size);

void serial_putc(unsigned char byte);
uint32_t serial_getc(void);
uint32_t serial_getc_noblock(void);
void serial_enable_interrupts(void);
int32_t serial_interrupt_handler(void);
