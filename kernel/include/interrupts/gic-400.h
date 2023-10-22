extern uint32_t num_interrupts;

/* CoreLink GIC-400 Generic Interrupt Controller */
/* based on code from: https://github.com/BrianSidebotham/arm-tutorial-rpi/blob/master/part-4/armc-013/gic-400.h */

/* Register bit definitions */
#define GIC400_CTL_ENABLE ( 1 << 0 )
#define GIC400_CTL_DISABLE ( 0 << 0 )

/** @brief Max interrupts that the GIC supports

    Indicates the maximum number of interrupts that the GIC supports. If ITLinesNumber=N, the
    maximum number of interrupts is 32(N+1). The interrupt ID range is from 0 to
    (number of IDs – 1). For example: 0b00011

    Up to 128 interrupt lines, interrupt IDs 0-127.

    The maximum number of interrupts is 1020 ( 0b11111 ). See the text in this section for more
    information.

    Regardless of the range of interrupt IDs defined by this field, interrupt IDs 1020-1023 are
    reserved for special purposes, see Special interrupt numbers on page 3-43 and Interrupt IDs
    on page 2-24.
*/
#define GIC400_TYPE_ITLINESNUMBER_GET(x) ( x & 0xF )

/*
    If the GIC implements the Security Extensions, the value of this field is the maximum number
    of implemented lockable SPIs, from 0 ( 0b00000 ) to 31 ( 0b11111 ), see Configuration lockdown
    on page 4-82. If this field is 0b00000 then the GIC does not implement configuration lockdown.

    If the GIC does not implement the Security Extensions, this field is reserved.
*/
#define GIC400_TYPE_LSPI_GET(x) ((x >> 11 ) & 0x1F)

/*
    Indicates whether the GIC implements the Security Extensions.
    0 - Security Extensions not implemented.
    1 - Security Extensions implemented.
*/
#define GIC400_TYPE_SECURITY_EXTENSIONS_GET(x) ((x >> 10) & 0x1)

/*
    Indicates the number of implemented CPU interfaces. The number of implemented CPU interfaces
    is one more than the value of this field, for example if this field is 0b011 , there are four
    CPU interfaces. If the GIC implements the Virtualization Extensions, this is also the number
    of virtual CPU interfaces.
*/
#define GIC400_TYPE_CPU_NUMBER_GET(x) ((x >> 5) & 0x7)

#define GIC400_TARGET_CPU0 ( 1 << 0 )
#define GIC400_TARGET_CPU1 ( 1 << 1 )
#define GIC400_TARGET_CPU2 ( 1 << 2 )
#define GIC400_TARGET_CPU3 ( 1 << 3 )
#define GIC400_TARGET_CPU4 ( 1 << 4 )
#define GIC400_TARGET_CPU5 ( 1 << 5 )
#define GIC400_TARGET_CPU6 ( 1 << 6 )
#define GIC400_TARGET_CPU7 ( 1 << 7 )

#define GIC400_ICFG_LEVEL_SENSITIVE ( 0 << 1 )
#define GIC400_ICFG_EDGE_TRIGGERED  ( 1 << 1 )





#define ARM_GICD_BASE	(0xFF840000UL+0x1000)
#define ARM_GICC_BASE	(0xFF840000UL+0x2000)

#define GICD_CTLR		(ARM_GICD_BASE + 0x000)
	#define GICD_CTLR_DISABLE	(0 << 0)
	#define GICD_CTLR_ENABLE	(1 << 0)
	// secure access
	#define GICD_CTLR_ENABLE_GROUP0	(1 << 0)
	#define GICD_CTLR_ENABLE_GROUP1	(1 << 1)
#define GICD_TYPE		(ARM_GICD_BASE + 0x004)

#define GICD_IGROUPR0		(ARM_GICD_BASE + 0x080)		// secure access for group 0

#define GICD_ISENABLER0		(ARM_GICD_BASE + 0x100)
// 104 - 13c

#define GICD_ICENABLER0		(ARM_GICD_BASE + 0x180)
#define GICD_ISPENDR0		(ARM_GICD_BASE + 0x200)

#define GICD_ICPENDR0		(ARM_GICD_BASE + 0x280)	// clear pending

#define GICD_ISACTIVER0		(ARM_GICD_BASE + 0x300)
#define GICD_ICACTIVER0		(ARM_GICD_BASE + 0x380)
#define GICD_IPRIORITYR0	(ARM_GICD_BASE + 0x400)
	#define GICD_IPRIORITYR_DEFAULT	0xA0
	#define GICD_IPRIORITYR_FIQ	0x40
#define GICD_ITARGETSR0		(ARM_GICD_BASE + 0x800)
	#define GICD_ITARGETSR_CORE0	(1 << 0)
#define GICD_ICFGR0		(ARM_GICD_BASE + 0xC00)
	#define GICD_ICFGR_LEVEL_SENSITIVE	(0 << 1)
	#define GICD_ICFGR_EDGE_TRIGGERED	(1 << 1)
#define GCID_PPISR		(ARM_GICD_BASE + 0xD00)
#define GCID_SPISR_0		(ARM_GICD_BASE + 0xD04)	// 0..31
#define GCID_SPISR_1		(ARM_GICD_BASE + 0xD08)	// 32..63
#define GCID_SPISR_2		(ARM_GICD_BASE + 0xD0C)	// 64..95 (ARMC)
#define GCID_SPISR_3		(ARM_GICD_BASE + 0xD10)	// 
#define GCID_SPISR_4		(ARM_GICD_BASE + 0xD14)
#define GCID_SPISR_5		(ARM_GICD_BASE + 0xD18)
#define GCID_SPISR_6		(ARM_GICD_BASE + 0xD1C)
#define GCID_SPISR_7		(ARM_GICD_BASE + 0xD20)
#define GCID_SPISR_8		(ARM_GICD_BASE + 0xD24)
#define GCID_SPISR_9		(ARM_GICD_BASE + 0xD28)
#define GCID_SPISR_10		(ARM_GICD_BASE + 0xD2C)
#define GCID_SPISR_11		(ARM_GICD_BASE + 0xD30)
#define GCID_SPISR_12		(ARM_GICD_BASE + 0xD34)
#define GCID_SPISR_13		(ARM_GICD_BASE + 0xD38)
#define GCID_SPISR_14		(ARM_GICD_BASE + 0xD3C)	// 448..479








#define GICD_SGIR		(ARM_GICD_BASE + 0xF00)
	#define GICD_SGIR_SGIINTID__MASK		0x0F
	#define GICD_SGIR_CPU_TARGET_LIST__SHIFT	16
	#define GICD_SGIR_TARGET_LIST_FILTER__SHIFT	24



// GIC CPU interface registers
#define GICC_CTLR		(ARM_GICC_BASE + 0x000)
	#define GICC_CTLR_DISABLE	(0 << 0)
	#define GICC_CTLR_ENABLE	(1 << 0)
	// secure access
	#define GICC_CTLR_ENABLE_GROUP0	(1 << 0)
	#define GICC_CTLR_ENABLE_GROUP1	(1 << 1)
	#define GICC_CTLR_FIQ_ENABLE	(1 << 3)
#define GICC_PMR		(ARM_GICC_BASE + 0x004)
	#define GICC_PMR_PRIORITY	(0xF0 << 0)
#define GICC_IAR		(ARM_GICC_BASE + 0x00C)
	#define GICC_IAR_INTERRUPT_ID__MASK	0x3FF
	#define GICC_IAR_CPUID__SHIFT		10
	#define GICC_IAR_CPUID__MASK		(3 << 10)
#define GICC_EOIR		(ARM_GICC_BASE + 0x010)
	#define GICC_EOIR_EOIINTID__MASK	0x3FF
	#define GICC_EOIR_CPUID__SHIFT		10
	#define GICC_EOIR_CPUID__MASK		(3 << 10)



extern uint32_t gic400_init(void* interrupt_controller_base);
