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
#you might want to comment this one out.
	$(CC) example/*.c main.c $(LIB) -o main_example.bin $(GL_INCLUDES) $(SDL_LIBS) $(SDL_MIXERLIBS) $(GL_LIBS) $(CFLAGS)
#this is the one you'll want to keep
	$(CC) *.c $(LIB) -o main $(GL_INCLUDES) $(SDL_LIBS) $(SDL_MIXERLIBS) $(GL_LIBS) $(CFLAGS)
