include ../config.mk
#CC= gcc
#CFLAGS = -g -Wall -O3 -w
GL_LIBS= -L../ 
GL_INCLUDES= -I../include/
ALL_T= main
LIB= ../lib/libTinyGL.a





all: $(ALL_T)
clean:
	rm -f $(ALL_T) *.exe
main:
	$(CC) *.c $(LIB) -o main $(GL_INCLUDES) $(SDL_LIBS) $(SDL_MIXERLIBS) $(GL_LIBS) $(CFLAGS)
