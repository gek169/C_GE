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
#define LUA_EXPORT(fname) int lua_##fname(lua_State* L)
#define LUA_IMPORT(fname) lua_register(L_STATE, #fname, lua_##fname);
#define LUA_FLOATARG(n) float arg##n = lua_tonumber(L, n);
#define LUA_INTARG(n) int arg##n = lua_tointeger(L, n);
#define LUA_STRINGARG(n) const char* arg##n = lua_tostring(L, n);
#define MAX_TRACKS 1000
#define MAX_SAMPS 1000
track* tracks[MAX_TRACKS] = {0};
samp* samps[MAX_SAMPS] = {0};
#define MAX_ENTITIES 0x10000
ChadWorld entity_world;
ChadEntity entities[MAX_ENTITIES];

//camera transforms.
vec3 campos;
vec3 camrot;
mat4 camproj;


/*LUA FUNCTIONS*/
LUA_EXPORT(drawBox){
	LUA_FLOATARG(1);
	LUA_FLOATARG(2);
	LUA_FLOATARG(3);
	LUA_FLOATARG(4);
	drawBox(arg1, arg2, arg3, arg4);
	return LUA_OK;
}
LUA_EXPORT(omg_box){
	//int omg_box(float x, float y, float xdim, float ydim, 
	//int sucks, float buttonjumpx, float buttonjumpy, int hints);
	LUA_FLOATARG(1);
	LUA_FLOATARG(2);
	LUA_FLOATARG(3);
	LUA_FLOATARG(4);
	LUA_INTARG(5); //sucks
	LUA_FLOATARG(6);
	LUA_FLOATARG(7);
	LUA_INTARG(8);
	lua_pushinteger(L, omg_box(arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8));
	return LUA_OK;
}

LUA_EXPORT(omg_textbox){
	//int omg_textbox(float x, float y, const char* text, 
	//int textsize, int sucks, float buttonjumpx, float buttonjumpy, int hints, int hintstext);
	LUA_FLOATARG(1);
	LUA_FLOATARG(2);
	LUA_STRINGARG(3);
	LUA_INTARG(4);
	LUA_INTARG(5);
	LUA_FLOATARG(6);
	LUA_FLOATARG(7);
	LUA_INTARG(8);
	LUA_INTARG(9);
	lua_pushinteger(L, omg_textbox(arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9));
	return LUA_OK;
}
int lua_loadTexture(lua_State* L){
	const char* texturename = lua_tostring(L, 1);
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

int lua_setCamPos(lua_State* L){
	campos.d[0] = lua_tonumber(L,1);
	campos.d[1] = lua_tonumber(L,2);
	campos.d[2] = lua_tonumber(L,3);
	return LUA_OK;
}
int lua_setCamRot(lua_State* L){
	camrot.d[0] = lua_tonumber(L,1);
	camrot.d[1] = lua_tonumber(L,2);
	camrot.d[2] = lua_tonumber(L,3);
	return LUA_OK;
}


int lua_resetProj(lua_State* L){
	(void)L;
	camproj = identitymat4();
	return LUA_OK;
}
int lua_setPerspective(lua_State* L){
	LUA_FLOATARG(1);
	LUA_FLOATARG(2);
	LUA_FLOATARG(3);
	LUA_FLOATARG(4);
	camproj = perspective(arg1,arg2,arg3,arg4);
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
int lua_set2D(lua_State* L){
	GLint arg = lua_tointeger(L, 1);
	if(arg) entity_world.world.is_2d = 1;
	else entity_world.world.is_2d = 0;
	return LUA_OK;
}

int lua_buildSpriteDisplayList(lua_State* L){
	LUA_FLOATARG(1); //width
	arg1/=2.0;
	LUA_FLOATARG(2); //height
	arg2/=2.0;
	LUA_INTARG(3); //texture ID.
	GLuint display_list = glGenLists(1);
	glNewList(display_list, GL_COMPILE);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, arg3);
		glColor3f(1,1,1);
		drawBox(-arg1/(float)winSizeX, -arg2/(float)winSizeY,
				arg1/(float)winSizeX * 2.0, arg2/(float)winSizeY * 2.0); //centered.
		glDisable(GL_TEXTURE_2D);
	glEndList();
	lua_pushinteger(L, display_list);
	return LUA_OK;
}

int lua_buildModelDisplayList(lua_State* L){
	const char* objname = lua_tostring(L, 1);
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
	lua_register(L_STATE, "set2D", lua_set2D);
	lua_register(L_STATE, "setCamPos", lua_setCamPos);
	lua_register(L_STATE, "setCamRot", lua_setCamRot);
	lua_register(L_STATE, "setPerspective", lua_setPerspective);
	LUA_IMPORT(drawBox);
	LUA_IMPORT(buildSpriteDisplayList);
	LUA_IMPORT(omg_box);
	LUA_IMPORT(omg_textbox);
	LUA_IMPORT(setTexturingEnabled);
	LUA_IMPORT(resetProj);
}

void draw_menu() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//TODO: invoke lua for draw menu.
	if (omg_textbox(0.01, 0.6, "\nQuit\n", 24, 1, 0.4, 0.2, 0xFFFFFF, 0) && omg_cb == 2) {
		puts("Quitting...");
		isRunning = 0;
	}
	drawMouse();
}

void draw_gameplay(){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//TODO: invoke lua for draw.
}
void draw(){
	if(is_in_menu) draw_menu();
	else draw_gameplay();
}

void initScene() {
	glClearColor(0.0, 0.0, 0.3, 0.0);
	glViewport(0, 0, winSizeX, winSizeY);
	SDL_ShowCursor(SDL_DISABLE);
	SDL_WM_SetCaption("Video Game", 0);
	{
		camproj = identitymat4();
		campos = (vec3){{0,0,0}};
		camrot = (vec3){{0,0,0}};
	}
	//TODO: invoke lua for initscene.
}


BEGIN_EVENT_HANDLER
//TODO: pass E_KEYSYM to lua.
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
