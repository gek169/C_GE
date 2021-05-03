/* OPENIMGUI STANDARD DEMO

Demo of Gek's proposed Open Immediate Mode Gui Standard



*/
//#define PLAY_MUSIC

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/GL/gl.h"

#include "../include/zbuffer.h"
#define CHAD_API_IMPL
#define CHAD_MATH_IMPL
#include "../include/3dMath.h"
#include "../include/api_audio.h"

#include <SDL/SDL.h>

// Gek's OpenIMGUI standard.
#define OPENIMGUI_IMPL
#include "../include/openimgui.h"

#ifndef M_PI
#define M_PI 3.14159265
#endif

int winSizeX = 640;
int winSizeY = 480;
double tpassed = 0;
int isRunning = 1;
int dirbstates[4] = {0, 0, 0, 0}; // up,down,left,right
int mousepos[2] = {0, 0};
int using_cursorkeys = 0; // Switches to cursor keys upon pressing a key.
int mb = 0;				  // cursor button
int mb2 = 0;			  // cursor second button.

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

vec3 mouse_to_normal(int mx, int my) {
	vec3 r;
	r.d[0] = mx / (float)winSizeX;
	r.d[1] = my / (float)winSizeY;
	return r;
}

void drawBox(GLfloat x, GLfloat y, GLfloat xdim, GLfloat ydim) { // 0,0 is top left, 1,1 is bottom right
	x *= 2;
	xdim *= 2;
	y *= 2;
	ydim *= 2;
	glBegin(GL_TRIANGLES);
	// TRIANGLE 1,
	glTexCoord2f(0, 0);
	glVertex3f(-1 + x, 1 - y - ydim, 0.5); // Bottom Left Corner

	glTexCoord2f(1, -1);
	glVertex3f(-1 + x + xdim, 1 - y, 0.5); // Top Right Corner

	glTexCoord2f(0, -1);
	glVertex3f(-1 + x, 1 - y, 0.5); // Top Left
	// TRIANGLE 2
	glTexCoord2f(0, 0);
	glVertex3f(-1 + x, 1 - y - ydim, 0.5); // Bottom Left Corner

	glTexCoord2f(1, 0);
	glVertex3f(-1 + x + xdim, 1 - y - ydim, 0.5);

	glTexCoord2f(1, -1);
	glVertex3f(-1 + x + xdim, 1 - y, 0.5); // Top Right Corner
	glEnd();
	return;
}

void drawMouse() {
	if (!omg_cb)
		glColor3f(0.7, 0.7, 0.7);
	else
		glColor3f(1.0, 0.1, 0.1);
	// if(!using_cursorkeys)
	drawBox(omg_cursorpos[0], omg_cursorpos[1], 0.03, 0.03);
	// else
	//	drawBox(omg_cursorpos_presuck[0],omg_cursorpos_presuck[1], 0.03, 0.03);
}

void drawTB(const char* text, GLuint textcolor, GLfloat x, GLfloat y, GLint size, float* tw, float* th) {
	size = (size > 64) ? 64 : ((size < 8) ? 8 : size);
	size >>= 3; // divide by 8 to get the GLTEXTSIZE
	if (!size || !text)
		return;
	int mw = 0, h = 1, cw = 0; // max width, height, current width
	for (int i = 0; text[i] != '\0' && (text[i] & 127); i++) {
		if (text[i] != '\n')
			cw++;
		else {
			cw = 0;
			h++;
		}
		if (mw < cw)
			mw = cw;
	}
	float w = (size)*8 * (mw) / (float)winSizeX;
	float bw = 3 * size / (float)winSizeX;
	float h_ = (size)*8 * (h) / (float)winSizeY;
	float bh = 3 * size / (float)winSizeY;
	drawBox(x, y, w, h_);
	*tw = w + bw;
	*th = h_ + bh;
	glTextSize(size);
	glDrawText((unsigned char*)text, x * winSizeX, y * winSizeY, textcolor);
	return;
}

int omg_box(float x, float y, float xdim, float ydim, int sucks, float buttonjumpx, float buttonjumpy, int hints) {
	// hints is the color of the box.
	float r = ((hints & 0xFF0000) >> 16) / 255.0;
	float g = ((hints & 0xFF00) >> 8) / 255.0;
	float b = ((hints & 0xFF)) / 255.0;
	glColor3f(r, g, b);
	drawBox(x, y, xdim, ydim);
	omg_box_suck(x, y, xdim, ydim, sucks, buttonjumpx, buttonjumpy);
	return omg_box_retval(x, y, xdim, ydim);
}

int omg_textbox(float x, float y, const char* text, int textsize, int sucks, float buttonjumpx, float buttonjumpy, int hints, int hintstext) {
	float r = ((hints & 0xFF0000) >> 16) / 255.0;
	float g = ((hints & 0xFF00) >> 8) / 255.0;
	float b = ((hints & 0xFF)) / 255.0;
	glColor3f(r, g, b);
	float xdim = 0, ydim = 0;
	drawTB(text, (GLuint)hintstext, x, y, textsize, &xdim, &ydim);
	omg_box_suck(x, y, xdim, ydim, sucks, buttonjumpx, buttonjumpy);
	return omg_box_retval(x, y, xdim, ydim);
}



#include "scene.h"





int main(int argc, char** argv) {
	// initialize SDL video:
	unsigned int fps = 0;
	char needsRGBAFix = 0;
	if (argc > 2) {
		char* larg = argv[1];
		for (int i = 0; i < argc; i++) {
			if (!strcmp(larg, "-w"))
				winSizeX = atoi(argv[i]);
			if (!strcmp(larg, "-h"))
				winSizeY = atoi(argv[i]);
			if (!strcmp(larg, "-fps"))
				fps = strtoull(argv[i], 0, 10);
			larg = argv[i];
		}
	}
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
		fprintf(stderr, "ERROR: cannot initialize SDL video and audio.\n");
		return 1;
	}

	ainit(0);
	SDL_Surface* screen = NULL;
	if ((screen = SDL_SetVideoMode(winSizeX, winSizeY, TGL_FEATURE_RENDER_BITS, SDL_SWSURFACE)) == 0) {
		fprintf(stderr, "ERROR: Video mode set failed.\n");
		return 1;
	}
#if TGL_FEATURE_RENDER_BITS == 32
	if (screen->format->Rmask != 0x00FF0000 || screen->format->Gmask != 0x0000FF00 || screen->format->Bmask != 0x000000FF) {
		needsRGBAFix = 1;
		printf("\nYour screen is using an RGBA output different than this library expects.");
		printf("\nYou should consider using the 16 bit version for optimal performance");
	}
#endif
	fflush(stdout);

	SDL_ShowCursor(SDL_ENABLE);
	SDL_WM_SetCaption(argv[0], 0);

	// initialize TinyGL:

	int mode;
	switch (screen->format->BitsPerPixel) {
	case 16:
		mode = ZB_MODE_5R6G5B;
		break;
	case 32:
		mode = ZB_MODE_RGBA;
		break;
	default:
		return 1;
		break;
	}
	ZBuffer* frameBuffer = ZB_open(winSizeX, winSizeY, mode, 0);
	glInit(frameBuffer);



	initScene();

	// variables for timing:
	long long unsigned int frames = 0;
	unsigned int tNow = SDL_GetTicks();
	unsigned int tLastFps = tNow;

	// main loop:

	while (isRunning) {
		++frames;
		tpassed += frames * 16.666666 / 1000.0;
		tNow = SDL_GetTicks();
		// do event handling:
		SDL_Event ev;
		while (SDL_PollEvent(&ev))
			events(&ev);
		if (using_cursorkeys)
			omg_update_keycursor(dirbstates[0], dirbstates[1], dirbstates[2], dirbstates[3], mb);
		else {
			vec3 r = mouse_to_normal(mousepos[0], mousepos[1]);
			omg_update_mcursor(r.d[0], r.d[1], mb);
		}
		// draw scene:
		
		// This is where we render our GUI!
		draw();
		if (SDL_MUSTLOCK(screen) && (SDL_LockSurface(screen) < 0)) {
			fprintf(stderr, "SDL ERROR: Can't lock screen: %s\n", SDL_GetError());
			return 1;
		}
		// Quickly convert all pixels to the correct format
#if TGL_FEATURE_RENDER_BITS == 32
		if (needsRGBAFix)
			for (int i = 0; i < frameBuffer->xsize * frameBuffer->ysize; i++) {
#define DATONE (frameBuffer->pbuf[i])
				DATONE = ((DATONE & 0x000000FF)) << screen->format->Rshift | ((DATONE & 0x0000FF00) >> 8) << screen->format->Gshift |
						 ((DATONE & 0x00FF0000) >> 16) << screen->format->Bshift;
			}
#endif
		ZB_copyFrameBuffer(frameBuffer, screen->pixels, screen->pitch);
		if (SDL_MUSTLOCK(screen))
			SDL_UnlockSurface(screen);
		SDL_Flip(screen);
		if (fps > 0)
			if ((1000 / fps) > (SDL_GetTicks() - tNow)) {
				SDL_Delay((1000 / fps) - (SDL_GetTicks() - tNow)); // Yay stable framerate!
			}
		// update fps:
	}
	//printf("%llu frames in %f secs, %f frames per second.\n", frames, (float)(tNow - tLastFps) * 0.001f, (float)frames * 1000.0f / (float)(tNow - tLastFps));
	// cleanup:
	ZB_close(frameBuffer);
	glClose();
	if (SDL_WasInit(SDL_INIT_VIDEO))
		SDL_QuitSubSystem(SDL_INIT_VIDEO);
	mhalt();
	//Mix_FreeMusic(myTrack);
	acleanup();
	SDL_Quit();
	return 0;
}
