struct sysinfo_t {
	int32_t total_processes;
	int32_t processes_ready;
	int32_t	total_ram;
	int32_t free_ram;
	int32_t uptime;
	int32_t idletime;
};

int32_t get_sysinfo(struct sysinfo_t *buf);
