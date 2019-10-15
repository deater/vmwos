/* Values from the BCM2835-ARM-Peripherals.pdf manual */

/* Note on Pi1 (systems with 512MB and less IO_BASE is 0x20000000 */
/* Pi2 and pi3 (systems with 1GB) IO_BASE is 0x3F000000 */
/* To confuse things, the GPU sees thing with IO_BAS 0x7e000000 */

/* We adjust the IO_BASE elsewhere */
#define	IO_BASE	0


/**************/
/* Interrupts */
/**************/
/* Section 7 */
			/*7e00b000 */
#define IRQ_BASE	IO_BASE+0x00b000

#define IRQ_BASIC_PENDING	(IRQ_BASE+0x200)

#define IRQ_BASIC_PENDING_TIMER		(1<<0)
#define IRQ_BASIC_PENDING_IRQ57		(1<<19)

#define IRQ_PENDING1		(IRQ_BASE+0x204)
#define IRQ_PENDING2		(IRQ_BASE+0x208)

#define IRQ_PENDING2_IRQ49	0x20000

#define IRQ_FIQ_CONTROL		(IRQ_BASE+0x20c)
#define IRQ_ENABLE_IRQ1		(IRQ_BASE+0x210)
#define IRQ_ENABLE_IRQ2		(IRQ_BASE+0x214)
#define IRQ_ENABLE_BASIC_IRQ	(IRQ_BASE+0x218)

#define IRQ_ENABLE_BASIC_IRQ_ACCESS_ERROR0	(1<<7)
#define IRQ_ENABLE_BASIC_IRQ_ACCESS_ERROR1	(1<<6)
#define IRQ_ENABLE_BASIC_IRQ_GPU1_HALTED	(1<<5)
#define IRQ_ENABLE_BASIC_IRQ_GPU0_HALTED	(1<<4)
#define IRQ_ENABLE_BASIC_IRQ_ARM_DOORBELL1	(1<<3)
#define IRQ_ENABLE_BASIC_IRQ_ARM_DOORBELL0	(1<<2)
#define IRQ_ENABLE_BASIC_IRQ_ARM_MAILBOX	(1<<1)
#define IRQ_ENABLE_BASIC_IRQ_ARM_TIMER		(1<<0)

#define IRQ_DISABLE_IRQ1	(IRQ_BASE+0x21c)
#define IRQ_DISABLE_IRQ2	(IRQ_BASE+0x220)
#define IRQ_DISABLE_BASIC_IRQ	(IRQ_BASE+0x224)



/*********/
/* Timer */
/*********/
/* Section 14 */
			/*7e00b000 */
#define TIMER_BASE	IO_BASE+0x00b000

/* LOAD = Value to Count Down from */
#define TIMER_LOAD	(TIMER_BASE+0x400)
/* VALUE = Current timer Value */
#define TIMER_VALUE	(TIMER_BASE+0x404)
/* CONTROL = Control Values */
#define TIMER_CONTROL	(TIMER_BASE+0x408)
#define TIMER_CONTROL_FREE_ENABLE	(1<<9)	/* Enable Free Counter */
#define TIMER_CONTROL_HALT		(1<<8)	/* Halt if processor halted */
#define TIMER_CONTROL_ENABLE		(1<<7)	/* Enable counter */
				/* 6 = ignored, always free-running */
#define TIMER_CONTROL_INT_ENABLE	(1<<5)	/* Enable interrupt */
#define TIMER_CONTROL_PRESCALE_1	(0<<2)
#define TIMER_CONTROL_PRESCALE_16	(1<<2)
#define TIMER_CONTROL_PRESCALE_256	(2<<2)
#define TIMER_CONTROL_32BIT		(1<<1)	/* Manual says 23bit? typo? */
				/* 0 = ignored, always wrapping */

#define TIMER_IRQ_CLEAR	(TIMER_BASE+0x40c)
#define TIMER_RAW_IRQ	(TIMER_BASE+0x410)
#define TIMER_MASKED_IRQ	(TIMER_BASE+0x414)
#define TIMER_RELOAD	(TIMER_BASE+0x418)
#define TIMER_PREDIVIDER	(TIMER_BASE+0x41c)
#define TIMER_FREE_RUNNING	(TIMER_BASE+0x420)

/************/
/* Watchdog */
/************/

#define PM_BASE (IO_BASE + 0x100000)

#define PM_RSTC	(PM_BASE + 0x1c)
#define PM_PASSWORD 			0x5a000000
#define PM_RSTC_WRCFG_FULL_RESET	0x00000020

#define PM_WDOG (PM_BASE + 0x24)


/***********************************************/
/* Clock Manager (see BCM2835 Peripherals 6.2) */
/*   note the PWM clocks are not defined there */
/*   those values come from the Linux driver.  */
/***********************************************/

#define CM_BASE	(IO_BASE+0x101000)
#define CM_GP0CTL	(CM_BASE+0x70)
#define CM_GP0DIV	(CM_BASE+0x74)
#define CM_GP1CTL	(CM_BASE+0x78)
#define CM_GP1DIV	(CM_BASE+0x7C)
#define CM_GP2CTL	(CM_BASE+0x80)
#define CM_GP2DIV	(CM_BASE+0x84)
#define CM_PWMCTL	(CM_BASE+0xA0)
#define CM_PWMDIV	(CM_BASE+0xA4)

#define CM_PASSWORD	0x5a000000
#define CM_CLOCK_ENABLE		0x10	// Enable The Clock Generator
#define CM_SRC_OSCILLATOR	0x01	// Clock Source = Oscillator
#define CM_SRC_TESTDEBUG0	0x02	// Clock Source = Test Debug 0
#define CM_SRC_TESTDEBUG1	0x03	// Clock Source = Test Debug 1
#define CM_SRC_PLLAPER		0x04	// Clock Source = PLLA Per
#define CM_SRC_PLLCPER		0x05	// Clock Source = PLLC Per
#define CM_SRC_PLLDPER		0x06	// Clock Source = PLLD Per
#define CM_SRC_HDMIAUX		0x07	// Clock Source = HDMI Auxiliary
#define CM_SRC_GND		0x08	// Clock Source = GND


/***************************/
/* Random Number Generator */
/***************************/

#define RNG_BASE		(IO_BASE+0x104000)
#define RNG_CTRL		(RNG_BASE+0x0)
#define RNG_STATUS		(RNG_BASE+0x4)
#define RNG_DATA		(RNG_BASE+0x8)
#define RNG_FF_THRESHOLD	(RNG_BASE+0xc)



/********/
/* GPIO */
/********/

#define GPIO_BASE	(IO_BASE+0x200000)
/* GPFSEL = Function Select Registers */
/*   Each reg 3 groups of 10 */
/*   000 = Input, 001 = Output */
/*   100=ALT0, 101=ALT1, 110=ALT2, 111=ALT3 011=ALT4, 010=ALT5 */
#define GPIO_GPFSEL0	(GPIO_BASE + 0x00)
#define GPIO_GPFSEL1	(GPIO_BASE + 0x04)
#define GPIO_GPFSEL2	(GPIO_BASE + 0x08)
#define GPIO_GPFSEL3	(GPIO_BASE + 0x0c)
#define GPIO_GPFSEL4	(GPIO_BASE + 0x10)
#define GPIO_GPFSEL5	(GPIO_BASE + 0x14)

#define	GPIO_GPFSEL_INPUT	0x0
#define	GPIO_GPFSEL_OUTPUT	0x1
#define	GPIO_GPFSEL_ALT0	0x4
#define	GPIO_GPFSEL_ALT1	0x5
#define	GPIO_GPFSEL_ALT2	0x6
#define	GPIO_GPFSEL_ALT3	0x7
#define	GPIO_GPFSEL_ALT4	0x3
#define	GPIO_GPFSEL_ALT5	0x2

/* GPSET = GPIO Set */
/*  SET0 to set GPIO 32-0  */
/*  SET1 to set GPIO 54-33 */
#define GPIO_GPSET0	(GPIO_BASE + 0x1c)
#define GPIO_GPSET1	(GPIO_BASE + 0x20)
/* GPCLR = GPIO Clear */
/*  CLR0 to clear GPIO 32-0  */
/*  CLR1 to clear GPIO 54-33 */
#define GPIO_GPCLR0	(GPIO_BASE + 0x28)
#define GPIO_GPCLR1	(GPIO_BASE + 0x2c)
/* GPLEV0 = Level Detect */
/*  Return actual value on the pins */
#define GPIO_GPLEV0	(GPIO_BASE + 0x34)
#define GPIO_GPLEV1	(GPIO_BASE + 0x38)
/* GPEDS = Event Detect */
/*  Set based on rising/falling event status */
/*  Can trigger an interrupt */
/*  Three interrupts; one for each bank plus one for "any" */
#define GPIO_GPEDS0	(GPIO_BASE + 0x40)
#define GPIO_GPEDS1	(GPIO_BASE + 0x44)
/* GPREN = Rising Edge Detect */
#define GPIO_GPREN0	(GPIO_BASE + 0x4c)
#define GPIO_GPREN1	(GPIO_BASE + 0x50)
/* GPFEN = Falling Edge Detect */
#define GPIO_GPFEN0	(GPIO_BASE + 0x58)
#define GPIO_GPFEN1	(GPIO_BASE + 0x5c)
/* GPHEN = High level detect */
#define GPIO_GPHEN0	(GPIO_BASE + 0x64)
#define GPIO_GPHEN1	(GPIO_BASE + 0x68)
/* GPHEN = Low level detect */
#define GPIO_GPLEN0	(GPIO_BASE + 0x70)
#define GPIO_GPLEN1	(GPIO_BASE + 0x74)
/* AREN = Asynchronous Rising Edge Detect */
#define GPIO_GPAREN0	(GPIO_BASE + 0x7c)
#define GPIO_GPAREN1	(GPIO_BASE + 0x80)
/* AREN = Asynchronous Falling Edge Detect */
#define GPIO_GPAFEN0	(GPIO_BASE + 0x88)
#define GPIO_GPAFEN1	(GPIO_BASE + 0x8c)
/* GPPUD = Pull Up/Down */
/* Set in conjunction with below */
/*   00 = disable, 01 = PullDown, 10 = PullUp */
#define GPIO_GPPUD	(GPIO_BASE + 0x94)
#define GPIO_GPPUD_DISABLE	0x0
#define GPIO_GPPUD_PULLDOWN	0x1
#define GPIO_GPPUD_PULLUP	0x2
/* To set the pullups: */
/* 1. Write to GPPUD to set the required control signal */
/*    Pull-up, Pull-Down, none                          */
/* 2. Wait 150 cycles (this provides required set-up time for signal */
/* 3. Write to GPPUDCLK0/1 to clock the control signal into the GPIO pads */
/*    you wish to modify (only pads which receive a clock will be modified) */
/* 4. Wait 150 cycles (this provides the required hold time) */
/* 5. Write to GPPUD to remove the control signal */
/* 6. Write to GPPUDCLK0/1 to remove the clock */
#define GPIO_GPPUDCLK0	(GPIO_BASE + 0x98)
#define GPIO_GPPUDCLK1	(GPIO_BASE + 0x9c)


/************************************************************************/
/* UART									*/
/************************************************************************/

			/*7e201000 */
#define UART0_BASE	IO_BASE+0x201000
/* DR = Data register */
/* On write, write 8-bits to send */
/* On receive, get 12-bits:       */
/*    Overrun Error == Overflowed FIFO */
/*    Break Error == data held low over full time */
/*    Parity Error */
/*    Frame Error == missing stop bit */
/* Then 8 bits of data */
#define UART0_DR	(UART0_BASE + 0x00)
/* RSRECR = Receive Status Register */
/*  Has same 4 bits of error as in DR */
#define UART0_RSRECR	(UART0_BASE + 0x04)
/* FR = Flags Register */
#define UART0_FR	(UART0_BASE + 0x18)
#define UART0_FR_TXFE	(1<<7)	/* TXFE = Transmit FIFO empty */
#define UART0_FR_RXFF	(1<<6)	/* RXFF = Receive FIFO full   */
#define UART0_FR_TXFF	(1<<5)	/* TXFF = Transmit FIFO full */
#define UART0_FR_RXFE	(1<<4)	/* RXFE = Receive FIFO empty */
#define UART0_FR_BUSY	(1<<3)	/* BUSY = UART is busy transmitting */
				/*  2 = DCD  = Unsupported */
				/*  1 = DSR  = Unsupported */
#define UART0_FR_CTS	(1<<0)	/* CTS  = inverted version of nUARTCTS value */

/* ILPR = Infrared, disabled on BC2835 */
#define UART0_ILPR	(UART0_BASE + 0x20)
/* IBRD = Integer part of Baud Rate Divisor */
/*    bottom 16 bits */
/*    UART must be disabled to change */
#define UART0_IBRD	(UART0_BASE + 0x24)
/* FBRD = Fractional part of Baud Rate Divisor */
/*   bottom 5 bits */
/*   Baud rate divisor BAUDDIV = (FUARTCLK/(16 Baud rate)) */
/*    UART must be disabled to change */
#define UART0_FBRD	(UART0_BASE + 0x28)
/* LCRH = Line Control Register */
#define UART0_LCRH	(UART0_BASE + 0x2C)
#define UART0_LCRH_SPS	(1<<7)	/* SPS = Stick Parity Select (0=disabled) */
				/* WLEN = word length 00=5, 01=6, 10=7, 11=8 */
#define UART0_LCRH_WLEN_5BIT	(0<<5)
#define UART0_LCRH_WLEN_6BIT	(1<<5)
#define UART0_LCRH_WLEN_7BIT	(2<<5)
#define UART0_LCRH_WLEN_8BIT	(3<<5)
#define UART0_LCRH_FEN	(1<<4)	/* FEN = enable FIFOs */
#define UART0_LCRH_STP2	(1<<3)	/* STP2 = enable 2 stop bits */
#define UART0_LCRH_EPS	(1<<2)	/* EPS  = even parity select */
#define UART0_LCRH_PEN	(1<<1)	/* PEN  = parity enable */
#define UART0_LCRH_BRK	(1<<0)	/* BRK  = send break after next character */

/* CR = Control Register */
/*   To enable transmission TXE and UARTEN must be set to 1 */
/*   To enable reception RXE and UARTEN must be set to 1 */
/*   Program the control registers as follows: */
/*    1. Disable the UART.                     */
/*    2. Wait for the end of transmission or   */
/*       reception of the current character.   */
/*    3. Flush the transmit FIFO by setting    */
/*       the FEN bit to 0 in the Line Control  */
/*       Register, UART_LCRH.                  */
/*    4. Reprogram UART_CR                     */
/*    5. Enable the UART.                      */
#define UART0_CR	(UART0_BASE + 0x30)
#define UART0_CR_CTSEN	(1<<15)	/*   15 = CTSEN = CTS Flow Enable */
#define UART0_CR_RTSEN	(1<<14)	/*   14 = RTSEN = RTS Flow Enable */
				/*   13 = OUT2 = Unsupported */
				/*   12 = OUT1 = Unsupported */
#define UART0_CR_RTS	(1<<11)	/*   11 = RTS = Request to Send */
					/*   10 = DTR = Unsupported */
#define UART0_CR_RXE	(1<<9)	/*    9 = RXE = Receive Enable */
#define UART0_CR_TXE	(1<<8)	/*    8 = TXE = Transmit Enable */
#define UART0_CR_LBE	(1<<7)	/*    7 = LBE = Loopback Enable */
				/*    6 - 3 = RESERVED */
				/*    2 = SIRLP = Unsupported */
				/*    1 = SIREN = Unsupported */
#define UART0_CR_UARTEN	(1<<0)	/*    0 = UARTEN = UART Enable */

/* IFLS = FIFO Level Select */
/*  11 - 9 = RXIFPSEL = Unsupported */
/*   8 - 6 = TXIFPSEL = Unsupported */
/*   5 - 3 = RXIFLSEL = 000=1/8, 001=1/4, 010=1/2, 011=3/4 100=7/8 */
/*   2 - 0 = TXIFLSEL = 000=1/8, 001=1/4, 010=1/2, 011=3/4 100=7/8 */
#define UART0_IFLS	(UART0_BASE + 0x34)

/* IMSRC = Interrupt Mask Set/Clear */
/*   10 = OEIM = Overrun Interrupt Mask */
/*    9 = BEIM = Break Interrupt Mask */
/*    8 = PEIM = Parity Interrupt Mask */
/*    7 = FEIM = Framing Interrupt Mask */
/*    6 = RTIM = Receivce Timeout Mask */
/*    5 = TXIM = Transmit Interrupt Mask */
/*    4 = RXIM = Receive Masked Interrupt Mask */
/*    3 = DSRMIM (unsupported) */
/*    2 = DCDMIM (unsupported) */
/*    1 = CTSMIM = nUARTCTS Mask */
/*    0 = RIMIM (unsupported) */
#define UART0_IMSC	(UART0_BASE + 0x38)
#define UART0_IMSC_OEIM	(1<<10) /* OERIS = Overrun Interrupt Raw Status */
#define UART0_IMSC_BEIM	(1<<9)  /* BERIS = Break Interrupt Raw Status*/
#define UART0_IMSC_PEIM	(1<<8)  /* PERIS = Parity Interrupt Raw Status */
#define UART0_IMSC_FEIM	(1<<7)	/* FERIS = Framing Interrupt Raw Status */
#define UART0_IMSC_RTIM	(1<<6)	/* RTRIS = Receivce Timeout Raw Status */
#define UART0_IMSC_TXIM	(1<<5)  /* TXRIS = Transmit Interrupt Raw Status */
#define UART0_IMSC_RXIM	(1<<4)	/* RXRIS = Receive Masked Interrupt Raw Status */
				/*    3 = DSRRIS (unsupported) */
				/*    2 = DCDRIS (unsupported) */
#define UART0_IMSC_CTSMIM	(1<<1)	/* CTSRIS = nUARTCTS Raw Status */
				/*    0 = RIRIS (unsupported) */


/* RIS = Raw Interrupt Status */
#define UART0_RIS	(UART0_BASE + 0x3C)
#define UART0_IMSC_OERIS	(1<<10) /* OERIS = Overrun Interrupt Raw Status */
#define UART0_IMSC_BERIS	(1<<9)  /* BERIS = Break Interrupt Raw Status*/
#define UART0_IMSC_PERIS	(1<<8)  /* PERIS = Parity Interrupt Raw Status */
#define UART0_IMSC_FERIS	(1<<7)	/* FERIS = Framing Interrupt Raw Status */
#define UART0_IMSC_RTRIS	(1<<6)	/* RTRIS = Receivce Timeout Raw Status */
#define UART0_IMSC_TXRIS	(1<<5)  /* TXRIS = Transmit Interrupt Raw Status */
#define UART0_IMSC_RXRIS	(1<<4)	/* RXRIS = Receive Masked Interrupt Raw Status */
				/*    3 = DSRRIS (unsupported) */
				/*    2 = DCDRIS (unsupported) */
#define UART0_IMSC_CTSRIS	(1<<1)	/* CTSRIS = nUARTCTS Raw Status */
				/*    0 = RIRIS (unsupported) */


/* MIS = Masked Interrupt Status */
/*   10 = OEMIS = Overrun Interrupt Masked Status */
/*    9 = BEMIS = Break Interrupt Masked Status*/
/*    8 = PEMIS = Parity Interrupt Masked Status */
/*    7 = FEMIS = Framing Interrupt Masked Status */
/*    6 = RTMIS = Receivce Timeout Masked Status */
/*    5 = TXMIS = Transmit Interrupt Masked Status */
/*    4 = RXMIS = Receive Masked Interrupt Masked Status */
/*    3 = DSRMIS (unsupported) */
/*    2 = DCDMIS (unsupported) */
/*    1 = CTSMIS = nUARTCTS Masked Status */
/*    0 = RIMIS (unsupported) */
#define UART0_MIS	(UART0_BASE + 0x40)
#define UART0_RXMIS	(1<<4)

/* ICR = Interrupt Clear Register */
/*   10 = OEIC = Overrun Interrupt Clear */
/*    9 = BEIC = Break Interrupt Clear */
/*    8 = PEIC = Parity Interrupt Clear */
/*    7 = FEIC = Framing Interrupt Clear */
/*    6 = RTIC = Receivce Timeout Interrupt Clear */
/*    5 = TXIC = Transmit Interrupt Clear */
/*    4 = RXIC = Receive Masked Interrupt Status */
/*    3 = DSRMIC (unsupported) */
/*    2 = DCDMIC (unsupported) */
/*    1 = CTSMIC = nUARTCTS status */
/*    0 = RIMIC (unsupported) */
#define UART0_ICR	(UART0_BASE + 0x44)
#define UART0_ICR_RXIC	(1<<4)
#define UART0_ICR_RTIC	(1<<6)

/* DMACR = DMA Control Register */
/*  This is disabled on the BCM2835 */
#define UART0_DMACR	(UART0_BASE + 0x48)
/* ITCR = Test Control Register */
#define UART0_ITCR	(UART0_BASE + 0x80)
/* ITIP = Test Control Register */
#define UART0_ITIP	(UART0_BASE + 0x84)
/* ITOP = Test Control Register */
#define UART0_ITOP	(UART0_BASE + 0x88)
/* TDR = Test Data Register */
#define UART0_TDR	(UART0_BASE + 0x8C)



/************************************************/
/* PWM (see BCM2835 Peripherals Chapter 9	*/
/*   Pulse-Width Modulation			*/
/************************************************/

#define PWM_BASE	(IO_BASE+0x20C000)

#define PWM_CONTROL	(PWM_BASE+0x00)
#define PWM_STATUS	(PWM_BASE+0x04)
#define PWM_DMAC	(PWM_BASE+0x08)
#define PWM0_RANGE	(PWM_BASE+0x10)
#define PWM0_DATA	(PWM_BASE+0x14)
#define PWM_FIFO_IN	(PWM_BASE+0x18)
#define PWM1_RANGE	(PWM_BASE+0x20)
#define PWM1_DATA	(PWM_BASE+0x24)

#define PWM_CONTROL_USE_FIFO1	(1<<13)
#define PWM_CONTROL_ENABLE1	(1<<8)
#define PWM_CONTROL_CLEAR_FIFO	(1<<6)
#define PWM_CONTROL_USE_FIFO0	(1<<5)
#define PWM_CONTROL_ENABLE0	(1<<0)


#define PWM_STATUS_STA2		0x400
#define PWM_STATUS_STA1		0x200
#define PWM_STATUS_BERR		0x100
#define PWM_STATUS_GAPO4	0x80
#define PWM_STATUS_GAPO3	0x40
#define PWM_STATUS_GAPO2	0x20
#define PWM_STATUS_GAPO1	0x10
#define PWM_STATUS_RERR1	0x8
#define PWM_STATUS_WERR1	0x4
#define PWM_STATUS_EMPTY	0x2
#define PWM_STATUS_FULL		0x1

#define PWM_DMAC_ENAB	0x80000000


/************************************************/
/* DMA (see BCM2835 Peripherals Chapter 4	*/
/*   DMA Controller				*/
/************************************************/

#define DMA0_BASE	(IO_BASE+0x7000)
#define DMA1_BASE	(IO_BASE+0x7100)
#define DMA2_BASE	(IO_BASE+0x7200)
#define DMA3_BASE	(IO_BASE+0x7300)
#define DMA4_BASE	(IO_BASE+0x7400)
#define DMA5_BASE	(IO_BASE+0x7500)
#define DMA6_BASE	(IO_BASE+0x7600)
#define DMA7_BASE	(IO_BASE+0x7700)
#define DMA8_BASE	(IO_BASE+0x7800)
#define DMA9_BASE	(IO_BASE+0x7900)
#define DMA10_BASE	(IO_BASE+0x7a00)
#define DMA11_BASE	(IO_BASE+0x7b00)
#define DMA12_BASE	(IO_BASE+0x7c00)
#define DMA13_BASE	(IO_BASE+0x7d00)
#define DMA14_BASE	(IO_BASE+0x7e00)
/* Not a typo, discontiguous */
#define DMA15_BASE	(IO_BASE+0xe05000)

#define DMA_CS		0x00	/* Control and Status */
#define DMA_CONBLK_AD	0x04	/* Control Block Address */
#define DMA_TI		0x08	/* CB Word 0 (Transfer Information) */
#define DMA_SOURCE_AD	0x0c	/* CB Word 1 (Source Address) */
#define DMA_DEST_AD	0x10	/* CB Word 2 (Destination Address) */
#define DMA_TXFR_LEN	0x14	/* CB Word 3 (Transfer Length) */
#define DMA_STRIDE	0x18	/* CB Word 4 (2D Stride) */
#define DMA_NEXTCONBK	0x1c	/* CB Word 5 (Next CB Address) */
#define DMA_DEBUG	0x20	/* Debug */

#define DMA_CS_DMA_ACTIVE			0x1 /* Activate the DMA */
#define DMA_CS_DMA_END				0x2 /* DMA End Flag */
#define DMA_CS_DMA_INT				0x4 /* Interrupt Status */
#define DMA_CS_DMA_DREQ				0x8 /* DREQ State */
#define DMA_CS_DMA_PAUSED			0x10 /* Paused State */
#define DMA_CS_DMA_DREQ_STOPS_DMA		0x20 /* Paused by DREQ State */
#define DMA_CS_DMA_WAITING_FOR_OUTSTANDING_WRITES	0x40
				/* Waiting for the Last Write to be Received */
#define DMA_CS_DMA_ERROR			0x100 /* Error */

#define DMA_TI_INTEN		0x1	/* Interrupt Enable */
#define DMA_TI_TDMODE		0x2	/* 2D Mode */
#define DMA_TI_WAIT_RESP	0x8	/* Wait for a Write Response */
#define DMA_TI_DEST_INC		0x10	/* Destination Address Increment */
#define DMA_TI_DEST_WIDTH	0x20	/* Destination Transfer Width */
#define DMA_TI_DEST_DREQ	0x40	/* Control Destination Writes w DREQ */
#define DMA_TI_DEST_IGNORE	0x80	/* Ignore Writes */
#define DMA_TI_SRC_INC		0x100	/* Source Address Increment */
#define DMA_TI_SRC_WIDTH	0x200	/* Source Transfer Width */
#define DMA_TI_SRC_DREQ		0x400	/* Control Source Reads with DREQ */
#define DMA_TI_SRC_IGNORE	0x800	/* Ignore Reads */

#define DMA_TI_PERMAP_0		0x0	/* Continuous Un-paced Transfer */
#define DMA_TI_PERMAP_1		0x10000 /* Peripheral Number 1 */
#define DMA_TI_PERMAP_2		0x20000 /* Peripheral Number 2 */
#define DMA_TI_PERMAP_3		0x30000 /* Peripheral Number 3 */
#define DMA_TI_PERMAP_4		0x40000 /* Peripheral Number 4 */
#define DMA_TI_PERMAP_5		0x50000 /* Peripheral Number 5 */


#define DMA_INT_STATUS	(IO_BASE+0x7ee0)
#define DMA_ENABLE	(IO_BASE+0x7ff0)

#define DMA_EN0		0x1
#define DMA_EN1		0x2
#define DMA_EN2		0x4
#define DMA_EN3		0x8
#define DMA_EN4		0x10
#define DMA_EN5		0x20
#define DMA_EN6		0x40
#define DMA_EN7		0x80
#define DMA_EN8		0x100
#define DMA_EN9		0x200
#define DMA_EN10	0x400
#define DMA_EN11	0x800
#define DMA_EN12	0x1000
#define DMA_EN13	0x2000
#define DMA_EN14	0x4000
