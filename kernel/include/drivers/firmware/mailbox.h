/* Info on the firmware memory address: */
/* See https://lists.denx.de/pipermail/u-boot/2015-March/208201.html */
/* Details by "popcorn mix" */
/* Summary:
	BCM2835/6/7 is a GPU with small ARM tacked to the side.
	GPU uses "bus" addresses, the CPU uses "physical" addresses.

	VC MMU: the coarse MMU used by arm for accessing GPU mem.
		Each page is 16M and there are 64 pages.
		This maps 30-bits of physical address to 32-bits of bus address.
	The GPU sets this up and the default mapping is:

	2835 (256/512MB systems):
		+ first 32 pages map physical addresses 0x00000000-0x1fffffff
 		  to bus addresses 0x40000000-0x5ffffffff
		+ The next page maps physical adddress 0x20000000-0x20ffffff
		  to bus addresses 0x7e000000 to 0x7effffff
	2836/2837 (1GB systems):
		+ first 63 pages map physical addresses 0x00000000-0x3effffff
		  to bus addresses 0xc0000000-0xfefffffff
		+ The next page maps physical adddress 0x3f000000-0x3fffffff
		  to bus addresses 0x7e000000 to 0x7effffff

	Bus address 0x7exxxxxx contains the peripherals

	The top 16M of SDRAM is not visible to the ARM due the mapping of
		the peripherals.
	The GPU and GPU peripherals (DMA) can see it as they use bus addresses.

	The bus address cache alias bits are:

	From the VideoCore processor:
	+ 0x0 L1 and L2 cache allocating and coherent
	+ 0x4 L1 non-allocating, but coherent. L2 allocating and coherent
	+ 0x8 L1 non-allocating, but coherent. L2 non-allocating, but coherent
	+ 0xc SDRAM alias. Cache is bypassed. Not L1 or L2 allocating or coherent

	From the GPU peripherals (note: all peripherals bypass the L1 cache)
	+ 0x0 Do not use
	+ 0x4 L1 non-allocating, and incoherent. L2 allocating and coherent.
	+ 0x8 L1 non-allocating, and incoherent. L2 non-allocating, but coherent
	+ 0xc SDRAM alias. Cache is bypassed. Not L1 or L2 allocating or coherent

	In general as long as VideoCore processor and GPU peripherals use
	the same alias everything works out.
	Mixing aliases requires flushing/invalidating for coherency and is
	generally avoided.

	On 2835 the ARM has a 16K L1 cache and no L2 cache.
	The GPU has a 128k L2 cache.
	The GPU's L2 cache is accessible from the ARM but it's not
	particularly close (i.e. not very fast).
	However mapping through the L2 allocating alias (0x4) was shown to be
	beneficial on 2835, so that is the alias we use.

	On 2836 the ARM has a 32K L1 cache and a 512k integrated/fast L2 cache.
	Going through the smaller/slower GPU L2 is bad for performance.
	So, we map through the SDRAM alias (0xc) and avoid the GPU L2 cache.

	If you don't use GPU peripherals or communicate with the GPU,
	you only care about physical addresses and it makes no difference
	what bus address is actually being used.
	The ARM just sees 1G of physical space that is always coherent.
	No flushing of GPU L2 cache is ever required.
	No need to know about aliases.

	If you do want to use GPU bus mastering peripherals (like DMA),
	or you communicate with the GPU (e.g. using the mailbox interface)
	you do need to distinguish physical and bus addresses, and you must
	use the correct alias.

	On 2835 you convert from physical to bus address with
		bus_address = 0x40000000 | physical_address;
	On 2836 you convert from physical to bus address with
		bus_address = 0xC0000000 | physical_address;

	Note: you can get these offsets from the device tree.

	When using GPU DMA, the addresses used for SCB, SA (source address),
	DA (dest address) must never be zero.
	They should be bus addresses and therefore 0x4 or 0xc aliases.
*/



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
#define MAILBOX_READ	(MAILBOX_BASE+0x00) /* 0xb880 4 Read Receiving mail. R */
#define MAILBOX_POLL	(MAILBOX_BASE+0x10) /* 0xb890 4 Poll Receive without retrieving. 	R */
#define MAILBOX_SENDER	(MAILBOX_BASE+0x14) /* 0xb894 4 Sender Sender information. 	R */
#define MAILBOX0_STATUS	(MAILBOX_BASE+0x18) /* 0xb898 4 Status Information. 	R */
#define MAILBOX_CONFIG	(MAILBOX_BASE+0x1c) /* 0xb89c 4 Configuration 	Settings. 	RW */
/* Mailbox 1 */
#define MAILBOX_WRITE	(MAILBOX_BASE+0x20) /* 0xb8a0 4 Write Sending mail. 	W  */
#define MAILBOX1_STATUS	(MAILBOX_BASE+0x38) /* 0xb8b8 4 Status Information. 	R */

int mailbox_write(unsigned int value, unsigned int channel);
int mailbox_read(unsigned int channel);
uint32_t firmware_phys_to_bus_address(uint32_t addr);
