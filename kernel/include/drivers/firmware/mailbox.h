#define MAILBOX_CHAN_POWER		0
#define MAILBOX_CHAN_FRAMEBUFFER	1
#define MAILBOX_CHAN_VIRT_UART		2
#define MAILBOX_CHAN_VCHIQ		3
#define MAILBOX_CHAN_LED		4
#define MAILBOX_CHAN_BUTTONS		5
#define MAILBOX_CHAN_TOUCHSCREEN	6
#define MAILBOX_CHAN_PROPERTY		8

#define MAIL_FULL	0x80000000
#define MAIL_EMPTY	0x40000000

#define MAILBOX_BASE	0xb880
/* Mailbox 0 */
#define MAILBOX_READ	(MAILBOX_BASE+0x00) /* 0x2000b880 4 Read Receiving mail. R */
#define MAILBOX_POLL	(MAILBOX_BASE+0x10) /* 0x2000b890 4 Poll Receive without retrieving. 	R */
#define MAILBOX_SENDER	(MAILBOX_BASE+0x14) /* 0x2000b894 4 Sender Sender information. 	R */
#define MAILBOX0_STATUS	(MAILBOX_BASE+0x18) /* 0x2000b898 4 Status Information. 	R */
#define MAILBOX_CONFIG	(MAILBOX_BASE+0x1c) /* 0x2000b89c 4 Configuration 	Settings. 	RW */
/* Mailbox 1 */
#define MAILBOX_WRITE	(MAILBOX_BASE+0x20) /* 0x2000b8a0 4 Write Sending mail. 	W  */
#define MAILBOX1_STATUS	(MAILBOX_BASE+0x38) /* 0x2000b8b8 4 Status Information. 	R */

int mailbox_write(unsigned int value, unsigned int channel);
int mailbox_read(unsigned int channel);
