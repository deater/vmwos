/* div.c */
uint32_t __aeabi_uidiv(uint32_t dividend, uint32_t divisor);
int32_t __aeabi_idiv(int32_t dividend, int32_t divisor);

/* error.c */
extern int errno;
char *strerror(int errnum);

/* stdio.c */
struct file_struct {
	int fd;
};
typedef struct file_struct FILE;
FILE stdin,stdout,stderr;

int putchar(int c);
int puts(char *s);
int getchar(void);
FILE *fopen(const char *pathname, const char *mode);
char *fgets(char *s, int size, FILE *stream);
int fclose(FILE *stream);

/* printf.c */
int printf(const char *string,...);
int sprintf(char *string, char *fmt, ...);
int fprintf(FILE *stream, const char *string, ...);

/* random.c */
int32_t rand(void);

/* string.c */
int32_t atoi(char *string);
int strlen(const char *s);
int strncmp(const char *s1, const char *s2, uint32_t n);
void *memset(void *s, int c, uint32_t n);

/* system.c */
int reboot(void);

/* time.c */
int32_t sleep(uint32_t seconds);
int32_t usleep(uint32_t usecs);
int time(int *time);
char *time_pretty(int32_t time, char *buffer, int32_t size);

/* tty.c */
void cfmakeraw(struct termios *termios_p);
