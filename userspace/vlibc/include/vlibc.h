unsigned int sleep(unsigned int seconds);
void cfmakeraw(struct termios *termios_p);

int putchar(int c);
int getchar(void);
int printf(char *string,...);
int time(int *time);
int reboot(void);
int errno;
int32_t rand(void);
int32_t atoi(char *string);



int strlen(const char *s);
int strncmp(const char *s1, const char *s2, uint32_t n);
char *strerror(int errnum);

char *time_pretty(int32_t time, char *buffer, int32_t size);
