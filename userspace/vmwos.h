#define __NR_blink	8192
#define __NR_setfont	8193
#define __NR_gradient		8194
#define __NR_tb1		8195
/* #define __NR_run		8196 obsolete */
/* #define __NR_stop		8197 obsolete */
#define __NR_temp		8198
#define __NR_random		8199
#define __NR_get_sysinfo	8200

int vmwos_blink(int value);
int vmwos_tb1(void);
int vmwos_setfont(int which);
int vmwos_gradient(void);
int vmwos_get_temp(void);
int vmwos_random(uint32_t *buffer);

struct sysinfo_t {
	int32_t total_processes;
	int32_t processes_ready;
	int32_t total_ram;
	int32_t free_ram;
	int32_t uptime;
	int32_t idletime;
};


int get_sysinfo(struct sysinfo_t *buffer);
