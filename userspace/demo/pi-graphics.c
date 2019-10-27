#include <stddef.h>
#include <stdint.h>

#include "svmwgraph.h"
#include "pi-graphics.h"

#include "syscalls.h"
#include "vmwos.h"
#include "vlibc.h"

//static const int xsize=XSIZE,ysize=YSIZE;

static unsigned char framebuffer[XSIZE*YSIZE*3];

int pi_graphics_update(unsigned char *buffer, struct palette *pal) {

	int x;

	/* BGR */
	for(x=0;x<XSIZE*YSIZE;x++) {
		framebuffer[x*3]=pal->blue[buffer[x]];
		framebuffer[(x*3)+1]=pal->green[buffer[x]];
		framebuffer[(x*3)+2]=pal->red[buffer[x]];
	}

	vmwos_framebuffer_load(XSIZE,YSIZE,24,framebuffer);

	return 0;
}

int pi_graphics_init(void) {

	return 0;
}

int pi_graphics_input(void) {

#ifdef VMWOS
	printf("Waiting 2s for input...\n");

	sleep(2);
#else
        SDL_Event event;
        int keypressed;


        while ( SDL_PollEvent(&event)) {

                switch(event.type) {

                case SDL_KEYDOWN:
                        keypressed=event.key.keysym.sym;
                        switch (keypressed) {

                        case SDLK_ESCAPE:
                                return 27;
                        case 'a'...'z':
                        case 'A'...'Z':
                                return keypressed;
                        case SDLK_UP:
                                return 11;
                        case SDLK_DOWN:
                                return 10;
                        case SDLK_RIGHT:
                                return 21;
                        case SDLK_LEFT:
                                return 8;
                        default:
                                printf("Unknown %d\n",keypressed);
                                return keypressed;
                        }
                        break;


                case SDL_JOYBUTTONDOWN:
                case SDL_JOYAXISMOTION:
                        printf("Joystick!\n");
                        break;

                default:
                        printf("Unknown input action!\n");
                        break;

                }
        }
#endif
        return 0;
}

