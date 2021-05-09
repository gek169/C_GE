#include "scene.h"
void drawMouse() {
	if (!omg_cb)
		glColor3f(0.7, 0.7, 0.7);
	else
		glColor3f(1.0, 0.1, 0.1);
	drawBox(omg_cursorpos[0], omg_cursorpos[1], 0.03, 0.03);
}
int haveclicked = 0; // For our toggleable movable button.
int is_in_menu = 0;
#define MAX_TRACKS 1000
#define MAX_SAMPS 1000
track* tracks[MAX_TRACKS] = {0};
samp* samps[MAX_SAMPS] = {0};
#define MAX_ENTITIES 0x10000
ChadWorld entity_world;
ChadEntity entities[MAX_ENTITIES];


/*LUA FUNCTIONS*/
int lua_loadTexture(lua_State* L){
	size_t len;
	const char* texturename = lua_tolstring(L, 1, &len);
	{
		int sw, sh, sc; GLint retval;
		uchar* source_data = stbi_load(texturename, &sw, &sh, &sc, 3);
		if(!source_data){
			lua_pushinteger(L, -1);
			printf("\nERROR!!! Cannot load '%s'!!!\n", texturename);
			return LUA_ERRERR;
		}
		retval = loadRGBTexture(source_data, sw, sh);
		free(source_data);
		lua_pushinteger(L, retval);
	}
	return LUA_OK;
}


int lua_bindTexture(lua_State* L){
	GLint texture_id = lua_tointeger(L, 1);
	glBindTexture(GL_TEXTURE_2D, texture_id);
	return LUA_OK;
}

int lua_deleteTexture(lua_State* L){
	GLuint texture_id = lua_tointeger(L, 1);
	glDeleteTextures(1, &texture_id);
	return LUA_OK;
}

int lua_deleteList(lua_State* L){
	GLint texture_id = lua_tointeger(L, 1);
	glDeleteLists(texture_id, 1);
	return LUA_OK;
}

int lua_callList(lua_State* L){
	GLint texture_id = lua_tointeger(L, 1);
	glCallList(texture_id);
	return LUA_OK;
}

int lua_setTexturingEnabled(lua_State* L){
	GLint arg = lua_tointeger(L, 1);
	if(arg) glEnable(GL_TEXTURE_2D);
	else glDisable(GL_TEXTURE_2D);
	return LUA_OK;
}

int lua_buildModelDisplayList(lua_State* L){
	size_t len;
	const char* objname = lua_tolstring(L, 1, &len);
	GLint texture_id = lua_tointeger(L, 2);
	{
		objraw omodel;
		model m = initmodel();
		omodel = tobj_load(objname);
		if (!omodel.positions){ //error.
			printf("\nERROR!!! Loading model '%s' results in ZERO POSITIONS!!!\n", objname);
			lua_pushinteger(L, -1);
			return LUA_ERRERR;
		}
		m = tobj_tomodel(&omodel);
		{
			GLint list;
			list = glGenLists(1);
			glNewList(list, GL_COMPILE);
			if(texture_id > 0){
				glEnable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D, texture_id);
			} else {
				glDisable(GL_TEXTURE_2D);
			}
			drawModel(m.d, m.npoints, m.c, m.n, m.t);
			glEndList();
			freemodel(&m);
			freeobjraw(&omodel);
			lua_pushinteger(L, list);
		}
	}
	return LUA_OK;
}

void createLuaBindings(){
	lua_register(L_STATE, "loadTexture", lua_loadTexture);
	lua_register(L_STATE, "buildModelDisplayList", lua_buildModelDisplayList);
	lua_register(L_STATE, "bindTexture", lua_bindTexture);
	lua_register(L_STATE, "callList", lua_callList);
	lua_register(L_STATE, "deleteList", lua_deleteList);
	lua_register(L_STATE, "deleteTexture", lua_deleteTexture);
	lua_register(L_STATE, "setTexturingEnabled", lua_setTexturingEnabled);
}

void draw_menu() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if (omg_textbox(0.01, 0.6, "\nQuit\n", 24, 1, 0.4, 0.2, 0xFFFFFF, 0) && omg_cb == 2) {
		puts("Quitting...");
		isRunning = 0;
	}
	drawMouse();
}

void draw_gameplay(){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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
	{

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
