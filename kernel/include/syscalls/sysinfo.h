/* Mostly compatible with the pretty awkward Linux interface */

struct sysinfo {
	int32_t uptime;		/* Seconds since boot */
	uint32_t loads[3];	/* 1, 5, and 15 min load average */
	uint32_t total_ram;	/* Total RAM */
	uint32_t free_ram;	/* Free RAM */
	uint32_t sharedram;	/* Shared RAM */
	uint32_t bufferram;	/* Buffer RAM */
	uint32_t totalswap;	/* Total swap */
	uint32_t freeswap;	/* Free swap */
	uint32_t procs;		/* Processes */
	uint32_t padding[2];	/* pad to 60-bytes */
	uint32_t mem_unit;	/* Units the above are given in */
	uint32_t procs_ready;	/* vmwos custom */
	uint32_t idle_time;	/* vmwos custom */
};

int32_t sysinfo(struct sysinfo *buf);
