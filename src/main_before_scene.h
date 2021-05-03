/* C_GE */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/GL/gl.h"

#include "../include/zbuffer.h"
#define STB_IMAGE_IMPLEMENTATION
#include "../include/stb_image.h"
#define CHAD_API_IMPL
#define CHAD_MATH_IMPL
#include "../include/3dMath.h"
#include "../include/tobjparse.h"
#include "../include/api_audio.h"

#include <SDL/SDL.h>

// Gek's OpenIMGUI standard.
#define OPENIMGUI_IMPL
#include "../include/openimgui.h"

#ifndef M_PI
#define M_PI 3.14159265358979
#endif

int winSizeX = 640;
int winSizeY = 480;
int isRunning = 1;
int dirbstates[4] = {0, 0, 0, 0}; // up,down,left,right
int mousepos[2] = {0, 0};
int using_cursorkeys = 0; // Switches to cursor keys upon pressing a key.
int mb = 0;				  // cursor button
int mb2 = 0;			  // cursor second button.


double tpassed = 0;

//uchar* source_data = stbi_load("tex_hole.png", &sw, &sh, &sc, 3);

GLuint loadRGBTexture(unsigned char* buf, unsigned int w, unsigned int h) {
	GLuint t = 0;
	glGenTextures(1, &t);
	glBindTexture(GL_TEXTURE_2D, t);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, buf);
	return t;
}

GLuint createModelDisplayList(
	// HUGE important note! these depend on the math library using
	// f_ as float and not double!
	// Remember that!
	vec3* points, uint npoints, vec3* colors, vec3* normals, vec3* texcoords) {
	GLuint ret = 0;
	if (!points)
		return 0;
	ret = glGenLists(1);
	glNewList(ret, GL_COMPILE);
	glBegin(GL_TRIANGLES);
	for (uint i = 0; i < npoints; i++) {
		if (colors) {
			glColor3f(colors[i].d[0], colors[i].d[1], colors[i].d[2]);
		}
		if (texcoords)
			glTexCoord2f(texcoords[i].d[0], texcoords[i].d[1]);
		if (normals)
			glNormal3f(normals[i].d[0], normals[i].d[1], normals[i].d[2]);
		glVertex3f(points[i].d[0], points[i].d[1], points[i].d[2]);
	}
	// printf("\ncreateModelDisplayList is not the problem.\n");
	glEnd();
	glEndList();
	return ret;
}

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

