include ../config.mk
#CC= gcc
#CFLAGS = -g -Wall -O3 -w
GL_LIBS= -L../ 
GL_INCLUDES= -I../include/
ALL_T= main
LIB= ../lib/libTinyGL.a ../lib/liblua.a





all: $(ALL_T)
clean:
	rm -f $(ALL_T) *.exe
main:
	$(CC) scene.c main.c $(LIB) -o main $(GL_INCLUDES) $(SDL_LIBS) $(SDL_MIXERLIBS) $(GL_LIBS) $(CFLAGS)
	$(CC) scene2.c main.c $(LIB) -o main2.bin $(GL_INCLUDES) $(SDL_LIBS) $(SDL_MIXERLIBS) $(GL_LIBS) $(CFLAGS)
