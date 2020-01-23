#include <stdint.h>
#include <stddef.h>

#include "lib/errors.h"
#include "lib/printk.h"
#include "lib/string.h"
#include "lib/smp.h"

#include "drivers/console/console_io.h"

#include "fs/files.h"
#include "fs/inodes.h"
#include "fs/superblock.h"

#include "fs/romfs/romfs.h"

#include "processes/process.h"

//static int debug=1;

struct superblock_t superblock_table[8];

extern int32_t root_dir;

int32_t mount_syscall(const char *source, const char *target,
	const char *filesystemtype, uint32_t mountflags,
	const void *data) {

	int32_t result=0;

	if (!strncmp(filesystemtype,"romfs",5)) {
		result=romfs_mount(&superblock_table[0]);
		if (result>=0) {
			root_dir=result;
			result=0;
		}
	}
	else {
		result=-ENODEV;
	}

	return result;
}

int32_t statfs_syscall(const char *path, struct vmwos_statfs *buf) {
	/* FIXME: lookup path */

	return romfs_statfs(&superblock_table[0],buf);
}
