/* C_GE */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef ENGINE_IMPL
#define STB_IMAGE_IMPLEMENTATION
#define CHAD_API_IMPL
#define CHAD_MATH_IMPL
#define OPENIMGUI_IMPL
#endif

#include "../include/GL/gl.h"
#include "../include/zbuffer.h"
#include "../include/stb_image.h"
#include "../include/chade.h"
#include "../include/tobjparse.h"
#include <SDL/SDL.h>
#include "../include/openimgui.h"
#include "../lua/lua.h"
#include "../lua/lauxlib.h"
#include "../lua/lualib.h"


#ifndef M_PI
#define M_PI 3.14159265358979
#endif


#ifdef ENGINE_IMPL
int winSizeX = 640;
int winSizeY = 480;
int isRunning = 1;
int dirbstates[4] = {0, 0, 0, 0}; // up,down,left,right
int sixbuttons[6] = {0,0,0,0,0,0}; //ASD ZXC, corresponding to the buttons on a controller.
int mousepos[2] = {0, 0};
int using_cursorkeys = 0; // Switches to cursor keys upon pressing a key.
int mb = 0;				  // cursor button
int mb2 = 0;			  // cursor second button.
double tpassed = 0;
//Declare variables here.
lua_State* L_STATE;

#else
extern int winSizeX, winSizeY, isRunning, dirbstates[4], using_cursorkeys, mousepos[2], mb, mb2;
extern double tpassed;
extern int sixbuttons[6];
//Predeclare variables here.
extern lua_State* L_STATE;
#endif

//predeclare functions here.
void events(SDL_Event* e);
void initScene();
void createLuaBindings();
void draw();
void cleanup();
int omg_textbox(float x, float y, const char* text, int textsize, int sucks, float buttonjumpx, float buttonjumpy, int hints, int hintstext);
int omg_box(float x, float y, float xdim, float ydim, int sucks, float buttonjumpx, float buttonjumpy, int hints);
void drawTB(const char* text, GLuint textcolor, GLfloat x, GLfloat y, GLint size, float* tw, float* th);
void drawBox(GLfloat x, GLfloat y, GLfloat xdim, GLfloat ydim);


//inline functions go here.
static inline vec3 mouse_to_normal(int mx, int my) {
	vec3 r;
	r.d[0] = mx / (float)winSizeX;
	r.d[1] = my / (float)winSizeY;
	return r;
}
//uchar* source_data = stbi_load("tex_hole.png", &sw, &sh, &sc, 3);

//useful macros go here.
#define BEGIN_EVENT_HANDLER                                                                                                                                    \
	void events(SDL_Event* e) {                                                                                                                                \
		switch (e->type) {
#define E_KEYSYM e->key.keysym.sym

#define END_EVENT_HANDLER                                                                                                                                      \
	}                                                                                                                                                          \
	}
#define EVENT_HANDLER events
#define E_MOTION e->motion
#define E_BUTTON e->button.button
#define E_WINEVENT e->window.event
#define E_WINW e->window.data1
#define E_WINH e->window.data2



