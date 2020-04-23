CC = gcc
CFLAGS = -O2 -Wall

SDL_LIBS= `sdl-config --libs`
SDL_INCLUDE= `sdl-config --cflags`

all:	hellmood_memories

hellmood_memories:	hellmood_memories.o
	$(CC) -o hellmood_memories hellmood_memories.o $(LFLAGS) $(SDL_LIBS)

hellmood_memories.o:	hellmood_memories.c
	$(CC) $(CFLAGS) $(SDL_INCLUDE) -c hellmood_memories.c

clean:	
	rm -f *~ *.o hellmood_memories
