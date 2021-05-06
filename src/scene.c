#include "scene.h"



void drawMouse() {
	if (!omg_cb)
		glColor3f(0.7, 0.7, 0.7);
	else
		glColor3f(1.0, 0.1, 0.1);
	drawBox(omg_cursorpos[0], omg_cursorpos[1], 0.03, 0.03);
}

int haveclicked = 0; // For our toggleable movable button.
vec3 tbcoords = (vec3){{0.4, 0.4, 0}};
vec3 slidcoords = (vec3){{0.1, 0.8, 0}};
float slidmoffset = 0;
int slidersliding = 0; // Is the slider being slid?
GLuint boing_display_list = 0;
GLuint boing_texture = 0;

int is_in_menu = 0;


track* myTrack = NULL;

void draw_menu() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if (mb2) { // Use an additional input to move gui elements for testing- right click moves the button.
		tbcoords.d[0] = omg_cursorpos[0];
		tbcoords.d[1] = omg_cursorpos[1];
		haveclicked = 0;
	}

	if (omg_textbox(0.01, 0, "\nEntry 1\n", 24, 1, 0.4, 0.2, 0xFFFFFF, 0) && omg_cb == 2)
		puts("Entry 1");
	if (omg_textbox(0.01, 0.2, "\nEntry 2\n", 24, 1, 0.4, 0.2, 0xFFFFFF, 0) && omg_cb == 2)
		puts("Entry 2");
	if (omg_textbox(0.01, 0.4, "\nEntry 3\n", 24, 1, 0.4, 0.2, 0xFFFFFF, 0) && omg_cb == 2)
		puts("Entry 3");
	if (omg_textbox(0.01, 0.6, "\nQuit\n", 24, 1, 0.4, 0.2, 0xFFFFFF, 0) && omg_cb == 2) {
		puts("Quitting...");
		isRunning = 0;
	}

	if (omg_textbox(tbcoords.d[0], tbcoords.d[1], "\nClick me and I toggle color!\n", 16, 1, 0.4, 0.3, 0xFFFFFF, haveclicked ? 0xFF0000 : 0x00) &&
		omg_cb == 1) {
		puts("Detected click! EVENT FIRED!\n");
		haveclicked = !haveclicked;
	}
	// A slider element
	if (omg_textbox(slidcoords.d[0], slidcoords.d[1], "\n Slider \n", 16, 1, 0.4, 0.3, 0xFFFFFF, haveclicked ? 0xFF0000 : 0x00) && omg_cb == 1) {
		slidersliding = 1;
		slidmoffset = omg_cursorpos[0] - slidcoords.d[0];
	}
	if (omg_cb == 2)
		slidersliding = 0;
	// Handle the slider sliding behavior.
	if (slidersliding) {
		if (using_cursorkeys) {
			if (omg_udlr[3]) {
				slidcoords.d[0] = clampf(slidcoords.d[0] + 0.05, 0.1, 0.7);
			}
			if (omg_udlr[2]) {
				slidcoords.d[0] = clampf(slidcoords.d[0] - 0.05, 0.1, 0.7);
			}
			omg_cursorpos[0] = slidcoords.d[0] + slidmoffset;
			omg_cursorpos[1] = slidcoords.d[1];
		} else {
			// Move the element to the cursorposition's x.
			slidcoords.d[0] = clampf(omg_cursorpos[0] - slidmoffset, 0.1, 0.7);
		}
		printf("Slider's value is %f\n", slidcoords.d[0]);
	}
	drawMouse();
}

void draw_gameplay(){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
		glTranslatef(0.2, -0.2, 0);
		glCallList(boing_display_list);
	glPopMatrix();
}

void draw(){
	if(is_in_menu) draw_menu();
	else draw_gameplay();
	
}

void initScene() {
	// initialize GL:
	glClearColor(0.0, 0.0, 0.3, 0.0);
	glViewport(0, 0, winSizeX, winSizeY);
	SDL_ShowCursor(SDL_DISABLE);
	SDL_WM_SetCaption("Video Game", 0);
	myTrack = lmus("WWGW.mp3");
	mplay(myTrack, -1, 1000);
	{
		int sw, sh, sc;
		uchar* source_data = stbi_load("boing.png", &sw, &sh, &sc, 3);
		if(!source_data){
			puts("Cannot load sprite."); exit(1);
		}
		boing_texture = loadRGBTexture(source_data, sw, sh);
		free(source_data);
		boing_display_list = glGenLists(1);
		glNewList(boing_display_list, GL_COMPILE);
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, boing_texture);
			glColor3f(1,1,1);
			drawBox(0,0,     0.2,0.2); //centered.
			glDisable(GL_TEXTURE_2D);
		glEndList();
	}
}


BEGIN_EVENT_HANDLER
	case SDL_KEYDOWN:
	using_cursorkeys = 1;
	switch (E_KEYSYM) {
		case SDLK_ESCAPE:
		case SDLK_q: is_in_menu = !is_in_menu; break;
		case SDLK_UP: 	dirbstates[0] = 1; break;
		case SDLK_DOWN: dirbstates[1] = 1; break;
		case SDLK_LEFT: dirbstates[2] = 1; break;
		case SDLK_RIGHT:dirbstates[3] = 1; break;
		case SDLK_SPACE:
		case SDLK_RETURN: mb = 1;break;
		default: break;
	}
	break;
	case SDL_KEYUP:
	using_cursorkeys = 1;
	switch (E_KEYSYM) {
		case SDLK_SPACE:
		case SDLK_RETURN: mb = 0; break;
		case SDLK_UP:	dirbstates[0] = 0;	break;
		case SDLK_DOWN:	dirbstates[1] = 0;	break;
		case SDLK_LEFT:	dirbstates[2] = 0;	break;
		case SDLK_RIGHT:dirbstates[3] = 0;	break;
		default: break;
	}
	break;
	case SDL_QUIT:
	isRunning = 0;
	break;
	case SDL_MOUSEBUTTONDOWN:
		if (E_BUTTON == SDL_BUTTON_LEFT) mb = 1;
		if (E_BUTTON == SDL_BUTTON_RIGHT) mb2 = 1;
	break;
	case SDL_MOUSEBUTTONUP:
		if (E_BUTTON == SDL_BUTTON_LEFT)mb = 0;
		if (E_BUTTON == SDL_BUTTON_RIGHT)mb2 = 0;
	break;
	case SDL_MOUSEMOTION:
		using_cursorkeys = 0;
		mousepos[0] = E_MOTION.x;
		mousepos[1] = E_MOTION.y;
	break;
END_EVENT_HANDLER
