#define __NR_blink	8192
#define __NR_setfont	8193
#define __NR_gradient		8194
#define __NR_tb1		8195
/* #define __NR_run		8196 obsolete */
/* #define __NR_stop		8197 obsolete */
#define __NR_temp		8198
#define __NR_random		8199
#define __NR_framebuffer_load	8200
#define __NR_malloc		8201
#define __NR_core_poke		8202
#define __NR_play_sound		8203

int vmwos_blink(int value);
int vmwos_tb1(void);
int vmwos_setfont(int which);
int vmwos_gradient(uint32_t type);
int vmwos_get_temp(void);
int vmwos_random(uint32_t *buffer);
int vmwos_framebuffer_load(int x, int y, int depth, unsigned char *fb);
void *vmwos_malloc(uint32_t size);
int vmwos_core_poke(uint32_t which);
int vmwos_play_sound(uint32_t *buffer, uint32_t length, uint32_t repeat);

