#define XSIZE	640
#define YSIZE	480

struct palette {
	unsigned char red[256];
	unsigned char green[256];
	unsigned char blue[256];
};

int pisim_update(unsigned char *buffer);
int pisim_init(void);
int pisim_input(void);

/* Font Routines */
#define DEFAULT_FONT	0

void *select_font(int which);
void vmwTextXY(char *string,int x,int y,int color,int background,int overwrite,
        int which_font, unsigned char *buffer);
void vmwTextXYx2(char *string,int x,int y,int color,int background,int overwrite,
        int which_font, unsigned char *buffer);
void vmwTextXYx4(char *string,int x,int y,int color,
        int which_font, unsigned char *buffer);
int put_char(unsigned char c, int x, int y, int fg_color, int bg_color,
        int overwrite, int which_font, unsigned char *buffer);
int put_char_cropped(unsigned char c, int x, int y, int fg_color, int bg_color,
        int overwrite, int which_font, unsigned char *buffer);
int put_charx4(unsigned char c, int x, int y, int fg_color,
        int which_font, unsigned char *buffer);
int print_string(char *string, int x, int y, int color,
	int which_font,unsigned char *buffer);

/* Palette */
void vmwFadeToBlack(unsigned char *buffer, struct palette *pal);
void vmwFadeFromBlack(unsigned char *buffer, struct palette *pal);
void vmwSetAllBlackPalette(struct palette *pal);

/* Clear Screen */
void vmwClearScreen(int color, unsigned char *buffer);
void vmwClearScreenY(int starty,int color, unsigned char *buffer);

/* Line Drawing */
void vmwHlin(int x1, int x2, int y, int color,unsigned char *buffer);
void vmwPlot(int x,int y, int color, unsigned char *buffer);
void vmwVlin(int y1, int y2, int x, int color, unsigned char *buffer);

/* Sprite */
void put_sprite_cropped(unsigned char *buffer,
			unsigned char *sprite,int x,int y);
void erase_sprite_cropped(unsigned char *buffer,
			unsigned char *sprite,int x,int y);

/* Apple2 Compatible */
void apple2_plot(int x, int y, int color, unsigned char *buffer);
void apple2_load_palette(struct palette *pal);

#define APPLE2_COLOR_BLACK		0
#define APPLE2_COLOR_DARKBLUE		2
#define APPLE2_COLOR_MEDIUMBLUE		6
#define APPLE2_COLOR_WHITE		15

/* console */
#define FORE_BLACK	0x0
#define FORE_BLUE	0x1
#define FORE_GREEN	0x2
#define FORE_CYAN	0x3
#define FORE_RED	0x4
#define FORE_PURPLE	0x5
#define FORE_BROWN	0x6
#define FORE_GREY	0x7
#define FORE_DGREY	0x8
#define FORE_LBLUE	0x9
#define FORE_LGREEN	0xa
#define FORE_LCYAN	0xb
#define FORE_LRED	0xc
#define FORE_PINK	0xd
#define FORE_YELLOW	0xe
#define FORE_WHITE	0xf

#define BACK_BLACK	0
#define BACK_BLUE	1
#define BACK_GREEN	2
#define BACK_CYAN	3
#define BACK_RED	4
#define BACK_PURPLE	5
#define BACK_BROWN	6
#define BACK_GREY	7

int console_write(const char *string, int length,
                unsigned char *buffer, struct palette *pal, int pi_top);
int console_init(struct palette *pal);
int console_clear(void);
int console_home(void);
int console_update(unsigned char *buffer, struct palette *pal, int pi_top);
int console_text_collapse(int starty,int how_long,
		unsigned char *buffer, struct palette *pal);
int console_text_explode(unsigned char *buffer, struct palette *pal);

/* pcx_load.h */
int vmwLoadPCX(unsigned char *image, int x, int y, unsigned char *buffer,
		int buffer_xsize);
int vmwPCXLoadPalette(unsigned char *image, int offset, struct palette *pal);
