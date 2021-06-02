#####################################################################
# C compiler
#####################################################################

#CC= clang
CC=cc
#CC=gcc
#CC=tcc


#For GCC/Clang on good OSes:
SDL_LIBS= -lSDL 
#for MinGW users:
#SDL_LIBS= -lmingw32 -lSDLmain -lSDL 

SDL_MIXERLIBS= -lSDL_mixer -lmad -logg -lmikmod -logg

#extract the maximum possible performance from gcc.
CFLAGS= -g -Wall -Ofast -std=c99 -DNDEBUG -fopenmp -mtune=native
CFLAGS_LIB= -g -Wall -Ofast -std=c99 -pedantic -DNDEBUG -fopenmp -mtune=native
#CFLAGS= -Wall -Ofast -std=gnu99 -DNDEBUG -mtune=native
#uncomment the following line for compiling the lib with tinyc
#keep these flags for compiling with gcc if you are linking with tinygl compiled with tcc.
#CFLAGS= -Wall -O3 -std=gnu99 -DNDEBUG -g
#CFLAGS= -Wall -O1 -g -std=c99 -Wno-undef -DNDEBUG
