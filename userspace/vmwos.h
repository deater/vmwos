int vmwos_blink(int value);
int vmwos_tb1(void);
int vmwos_setfont(int which);
int vmwos_gradient(void);

#define __NR_blink	8192
#define __NR_setfont	8193
#define __NR_gradient	8194
#define __NR_tb1	8195
#define __NR_run	8196

