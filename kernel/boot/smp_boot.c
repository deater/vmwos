#include "boot/smp_boot.h"
#include "lib/printk.h"

/* Boot up secondary cores */

void secondary_boot_c(int core) {

	/* Set up cache */

	printk("Booting core %d\n",core);

	while(1) {

	}

}
