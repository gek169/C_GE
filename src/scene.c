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
static inline void PushFloatArray(lua_State* L, float *array, int size){
    lua_newtable(L);
    for (int i = 0; i < size; i++)    {
            lua_pushinteger(L, i);
            lua_pushnumber(L, array[i]); // push the value at 'i'
            // the table is now at -3 on the stack. This tells Lua
            // to put "stack[-1]" at "stack[-2]" and pop them off,        
            // leaving the table at the top of the stack
           lua_settable(L, -3);    
    }
}
static inline void PushLongArray(lua_State* L, long *array, int size){
    lua_newtable(L);
    for (int i = 0; i < size; i++)    {
            lua_pushinteger(L, i);
            lua_pushinteger(L, array[i]); // push the value at 'i'
            // the table is now at -3 on the stack. This tells Lua
            // to put "stack[-1]" at "stack[-2]" and pop them off,        
            // leaving the table at the top of the stack
           lua_settable(L, -3);    
    }
}
static inline float popFloatFromArray(lua_State* L, int index){
	if(lua_istable(L, -1)){
		float result;
		lua_pushinteger(L, index);
		lua_gettable(L, -2);
		result = lua_tonumber(L, -1);
		lua_pop(L, 1);
		return result;
	}
	return 0.0;
}
static inline void popMat4fromArray(lua_State* L,mat4* dest){
	for(long i = 0; i < 16; i++)
		dest->d[i] = popFloatFromArray(L, 1 + i);
}
static inline void popFloatArray(lua_State* L, float* dest, long count){
	for(long i = 0; i < count; i++)
		dest[i] = popFloatFromArray(L, 1 + i);
}
static inline void popVec3fromArray(lua_State* L,vec3* dest){
	for(long i = 0; i < 3; i++)
		dest->d[i] = popFloatFromArray(L, 1 + i);
}
static inline void popVec4fromArray(lua_State* L,vec4* dest){
	for(long i = 0; i < 4; i++)
		dest->d[i] = popFloatFromArray(L, 1 + i);
}
static inline long popIntFromArray(lua_State* L, int index){
	if(lua_istable(L, -1)){
		long result;
		lua_pushinteger(L, index);
		lua_gettable(L, -2);
		result = lua_tointeger(L, -1);
		lua_pop(L, 1);
		return result;
	}
	return 0;
}
#define LUA_EXPORT(fname) static int lua_##fname(lua_State* L)
#define LUA_IMPORT(fname) lua_register(L_STATE, #fname, lua_##fname);
#define LUA_FLOATARG(n) float arg##n = lua_tonumber(L, n);
#define LUA_INTARG(n) int arg##n = lua_tointeger(L, n);
#define LUA_STRINGARG(n) const char* arg##n = lua_tostring(L, n);
#define LUA_FLOATPUSH(n) lua_pushnumber(L, n);
#define LUA_INTPUSH(n) 	lua_pushinteger(L, n);
#define LUA_STRINGPUSH(n) lua_pushstring(L, n);
#define LUA_INTGLOBSET(name) lua_pushinteger(L_STATE, name); lua_setglobal(L_STATE, #name);
#define LUA_FLOATGLOBSET(name) lua_pushnumber(L_STATE, name); lua_setglobal(L_STATE, #name);
#define LUA_STRINGGLOBSET(name) lua_pushstring(L_STATE, name); lua_setglobal(L_STATE, #name);
#define MAX_TRACKS 2048
#define MAX_SAMPS 2048
track* tracks[MAX_TRACKS] = {0};
samp* samps[MAX_SAMPS] = {0};
#define MAX_ENTITIES 0x10000
ChadWorld entity_world;
ChadEntity entities[MAX_ENTITIES];

//camera transforms.
mat4 camview;
mat4 camproj;
//Math registers
mat4 mat4regs[20];
vec3 vec3regs[20];
vec4 vec4regs[20];

/*LUA FUNCTIONS*/
LUA_EXPORT(glClearColor){
	vec4 temp;
	popVec4fromArray(L, &temp);
	glClearColor(temp.d[0],	temp.d[1], temp.d[2], temp.d[3]);
	return 0;
}
LUA_EXPORT(glClear){
	(void)L;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	return 0;
}
LUA_EXPORT(glTranslatef){
	vec3 temp;
	popVec3fromArray(L, &temp);
	glTranslatef(	temp.d[0],
					temp.d[1],
					temp.d[2]);
	return 0;
}
LUA_EXPORT(glRotate3f){
	vec3 temp;
	popVec3fromArray(L, &temp);
	glRotatef(	temp.d[0],
				temp.d[1],
				temp.d[2], 0);
	return 0;
}
LUA_EXPORT(glScalef){
	vec3 temp;
	popVec3fromArray(L, &temp);
	glScalef(	temp.d[0],
				temp.d[1],
				temp.d[2]);
	return 0;
}
LUA_EXPORT(glPushMatrix){
	(void)L;
	glPushMatrix();
	return 0;
}
LUA_EXPORT(glMultMatrixf){
	mat4 temp;
	popMat4fromArray(L, &temp);
	glMultMatrixf(temp.d);
	return 0;
}
LUA_EXPORT(glPopMatrix){
	(void)L;
	glPopMatrix();
	return 0;
}
LUA_EXPORT(mov_v3){
	LUA_INTARG(1); LUA_INTARG(2);
	vec3regs[arg1] = vec3regs[arg2];
	return 0;
}
LUA_EXPORT(mov_v4){
	LUA_INTARG(1); LUA_INTARG(2);
	vec4regs[arg1] = vec4regs[arg2];
	return 0;
}
LUA_EXPORT(mov_mat4){
	LUA_INTARG(1); LUA_INTARG(2);
	mat4regs[arg1] = mat4regs[arg2];
	return 0;
}
LUA_EXPORT(multm4){
	mat4 temp1;
	mat4 temp2;
	popMat4fromArray(L, &temp1);
	lua_pop(L, 1);
	popMat4fromArray(L, &temp2);
	temp1 = multm4(temp1, temp2);
	PushFloatArray(L, temp1.d, 16);
	return 1;
}
LUA_EXPORT(setEnableDepthTest){
	LUA_INTARG(1);
	if(arg1) glEnable(GL_DEPTH_TEST);
	else	glDisable(GL_DEPTH_TEST);
	return 0;
}
LUA_EXPORT(setEnableLighting){
	LUA_INTARG(1);
	if(arg1) glEnable(GL_LIGHTING);
	else	glDisable(GL_LIGHTING);
	return 0;
}
LUA_EXPORT(setCullingMode){
	LUA_INTARG(1);
	if(arg1) 
	{	LUA_INTARG(2);
		glEnable(GL_CULL_FACE);
		if(arg2) glCullFace(GL_BACK);
		else glCullFace(GL_FRONT);
	}
	else	glDisable(GL_CULL_FACE);
	return 0;
}
LUA_EXPORT(setEnableColorMaterial){
	LUA_INTARG(1);
	if(arg1) {glEnable(GL_COLOR_MATERIAL);}
	else	{glDisable(GL_COLOR_MATERIAL);}
	return 0;
}
LUA_EXPORT(setEnableLight){
	LUA_INTARG(1); //the light id.
	LUA_INTARG(2); //the light toggle
	if(arg2) {glEnable(GL_LIGHT0 + arg1);}
	else	{glDisable(GL_LIGHT0 + arg1);}
	return 0;
}
LUA_EXPORT(setLightProps){
	LUA_INTARG(1); //light id.
	LUA_INTARG(2); //property to set.
	GLfloat arr[4];
	if(arg2 == 0){
		LUA_FLOATARG(3);
		LUA_FLOATARG(4);
		LUA_FLOATARG(5);
		LUA_FLOATARG(6);
		arr[0] = arg3; arr[1] = arg4; arr[2] = arg5; arr[3] = arg6;
		glLightfv(GL_LIGHT0 + arg1, GL_AMBIENT, arr);
		return 0;
	} else if(arg2 == 1){
		LUA_FLOATARG(3);
		LUA_FLOATARG(4);
		LUA_FLOATARG(5);
		LUA_FLOATARG(6);
		arr[0] = arg3; arr[1] = arg4; arr[2] = arg5; arr[3] = arg6;
		glLightfv(GL_LIGHT0 + arg1, GL_DIFFUSE, arr);
		return 0;
	}else if(arg2 == 2){
		LUA_FLOATARG(3);
		LUA_FLOATARG(4);
		LUA_FLOATARG(5);
		LUA_FLOATARG(6);
		arr[0] = arg3; arr[1] = arg4; arr[2] = arg5; arr[3] = arg6;
		glLightfv(GL_LIGHT0 + arg1, GL_SPECULAR, arr);
		return 0;
	}else if(arg2 == 3){
		LUA_FLOATARG(3);
		LUA_FLOATARG(4);
		LUA_FLOATARG(5);
		LUA_FLOATARG(6);
		arr[0] = arg3; arr[1] = arg4; arr[2] = arg5; arr[3] = arg6;
		glLightfv(GL_LIGHT0 + arg1, GL_POSITION, arr);
		return 0;
	}else if(arg2 == 4){
		LUA_FLOATARG(3);
		LUA_FLOATARG(4);
		LUA_FLOATARG(5);
		//LUA_FLOATARG(6);
		arr[0] = arg3; arr[1] = arg4; arr[2] = arg5; //arr[3] = arg6;
		glLightfv(GL_LIGHT0 + arg1, GL_SPOT_DIRECTION, arr);
		return 0;
	}else if(arg2 == 5){
			LUA_FLOATARG(3);
			//LUA_FLOATARG(4);
			//LUA_FLOATARG(5);
			//LUA_FLOATARG(6);
			arr[0] = arg3; //arr[1] = arg4; arr[2] = arg5; arr[3] = arg6;
			glLightfv(GL_LIGHT0 + arg1, GL_SPOT_EXPONENT, arr);
			return 0;
	}else if(arg2 == 6){
		LUA_FLOATARG(3);
		//LUA_FLOATARG(4);
		//LUA_FLOATARG(5);
		//LUA_FLOATARG(6);
		arr[0] = arg3; //arr[1] = arg4; arr[2] = arg5; arr[3] = arg6;
		glLightfv(GL_LIGHT0 + arg1, GL_SPOT_CUTOFF, arr);
		return 0;
	}else if(arg2 == 7){
		LUA_FLOATARG(3);
		//LUA_FLOATARG(4);
		//LUA_FLOATARG(5);
		//LUA_FLOATARG(6);
		arr[0] = arg3; //arr[1] = arg4; arr[2] = arg5; arr[3] = arg6;
		glLightfv(GL_LIGHT0 + arg1, GL_CONSTANT_ATTENUATION, arr);
		return 0;
	}else if(arg2 == 8){
		LUA_FLOATARG(3);
		//LUA_FLOATARG(4);
		//LUA_FLOATARG(5);
		//LUA_FLOATARG(6);
		arr[0] = arg3; //arr[1] = arg4; arr[2] = arg5; arr[3] = arg6;
		glLightfv(GL_LIGHT0 + arg1, GL_LINEAR_ATTENUATION, arr);
		return 0;
	}else if(arg2 == 9){
		LUA_FLOATARG(3);
		//LUA_FLOATARG(4);
		//LUA_FLOATARG(5);
		//LUA_FLOATARG(6);
		arr[0] = arg3; //arr[1] = arg4; arr[2] = arg5; arr[3] = arg6;
		glLightfv(GL_LIGHT0 + arg1, GL_QUADRATIC_ATTENUATION, arr);
		return 0;
	}

	return 0;
}
LUA_EXPORT(setColorMaterialMode){
	LUA_INTARG(1);
	LUA_INTARG(2);
	if(arg1 == 0){
		arg1 = GL_FRONT;
	} else if (arg1 == 1){
		arg1 = GL_BACK;
	} else
		arg1 = GL_FRONT_AND_BACK;
	if(arg2 == 0){
		arg2 = GL_AMBIENT_AND_DIFFUSE;
	} else if (arg2 == 1){
		arg2 = GL_DIFFUSE;
	} else if (arg2 == 3){
		arg2 = GL_AMBIENT;
	} else if (arg2 == 4){
		arg2 = GL_SPECULAR;
	} else arg2 = GL_EMISSION;
	glColorMaterial(arg1, GL_AMBIENT_AND_DIFFUSE);
	return 0;
}
LUA_EXPORT(invmat4){
	mat4 temp;
	popMat4fromArray(L, &temp);
	mat4 out;
	invmat4(temp, &out);
	PushFloatArray(L, out.d, 16);
	return 1;
}
LUA_EXPORT(addv3){
	vec3 temp1;
	vec3 temp2;
	popVec3fromArray(L, &temp1);
	lua_pop(L,1);
	popVec3fromArray(L, &temp2);
	temp1 = addv3(temp1, temp2);
	PushFloatArray(L, temp1.d, 3);
	return 1;
}
LUA_EXPORT(subv3){
	vec3 temp1;
	vec3 temp2;
	popVec3fromArray(L, &temp1);
	lua_pop(L,1);
	popVec3fromArray(L, &temp2);
	temp1 = subv3(temp1, temp2);
	PushFloatArray(L, temp1.d, 3);
	return 1;
}
LUA_EXPORT(addv4){
	vec4 temp1;
	vec4 temp2;
	popVec4fromArray(L, &temp1);
	lua_pop(L,1);
	popVec4fromArray(L, &temp2);
	temp1 = addv4(temp1, temp2);
	PushFloatArray(L, temp1.d, 4);
	return 1;
}
LUA_EXPORT(subv4){
	vec4 temp1;
	vec4 temp2;
	popVec4fromArray(L, &temp1);
	lua_pop(L,1);
	popVec4fromArray(L, &temp2);
	temp1 = subv4(temp1, temp2);
	PushFloatArray(L, temp1.d, 4);
	return 1;
}
LUA_EXPORT(ld_vec3){
	LUA_INTARG(1);
	vec3 temp;
	popVec3fromArray(L, &temp);
	vec3regs[arg1] = temp;
	return 0;
}
LUA_EXPORT(st_vec3){
	LUA_INTARG(1);
	PushFloatArray(L, vec3regs[arg1].d, 3);
	return 1;
}
LUA_EXPORT(ld_vec4){
	LUA_INTARG(1);
	vec4 temp;
	popVec4fromArray(L, &temp);
	vec4regs[arg1] = temp;
	return 0;
}
LUA_EXPORT(st_vec4){
	LUA_INTARG(1);
	PushFloatArray(L, vec4regs[arg1].d, 4);
	return 1;
}
LUA_EXPORT(ld_mat4){
	LUA_INTARG(1); //register ID;
	mat4 temp;
	popMat4fromArray(L, &temp);
	mat4regs[arg1] = temp;
	return 0;
}
LUA_EXPORT(st_mat4){
	LUA_INTARG(1);
	PushFloatArray(L, mat4regs[arg1].d, 16);
	return 1;
}
LUA_EXPORT(dotv3){
	vec3 temp1;
	vec3 temp2;
	popVec3fromArray(L, &temp1);
	lua_pop(L,1);
	popVec3fromArray(L, &temp2);
	GLfloat result = dotv3(temp1, temp2);
	LUA_FLOATPUSH(result);
	return 1;
}
LUA_EXPORT(scalev3){
	LUA_FLOATARG(1);
	vec3 temp1;
	vec3 temp2;
	popVec3fromArray(L, &temp1);
	temp2 = scalev3(arg1, temp1);
	PushFloatArray(L, temp2.d, 3);
	return 1;
}
LUA_EXPORT(scalev4){
	LUA_FLOATARG(1);
	vec4 temp1;
	vec4 temp2;
	popVec4fromArray(L, &temp1);
	temp2 = scalev4(arg1, temp1);
	PushFloatArray(L, temp2.d, 4);
	return 1;
}
LUA_EXPORT(dotv4){
	vec4 temp1;
	vec4 temp2;
	popVec4fromArray(L, &temp1);
	lua_pop(L,1);
	popVec4fromArray(L, &temp2);
	GLfloat result = dotv4(temp1, temp2);
	LUA_FLOATPUSH(result);
	return 1;
}
LUA_EXPORT(stepChadWorld){
	LUA_INTARG(1);
	stepChadWorld(&entity_world, arg1);
	return 0;
}
LUA_EXPORT(renderChadWorld){
	(void)L;
	renderChadWorld(&entity_world);
	return 0;
}
LUA_EXPORT(entity_setDL){
	LUA_INTARG(1); //entity id
	LUA_INTARG(2); //display list
	entities[arg1].dl = arg2;
	return 0;
}
LUA_EXPORT(entity_setLocalT){
	LUA_INTARG(1); //entity id
	mat4 temp;
	popMat4fromArray(L, &temp);
	entities[arg1].body.localt = temp;
	return 0;
}
LUA_EXPORT(entity_setMass){
	LUA_INTARG(1); //entity id
	LUA_FLOATARG(2); //mass
	entities[arg1].body.mass = arg2;
	return 0;
}
LUA_EXPORT(entity_setVelocity){
	LUA_INTARG(1); //entity id
	vec3 temp;
	popVec3fromArray(L, &temp);
	entities[arg1].body.v = temp;
	return 0;
}
LUA_EXPORT(entity_getVelocity){
	LUA_INTARG(1); //entity id
	PushFloatArray(L, entities[arg1].body.v.d, 3);
	return 1;
}
LUA_EXPORT(entity_getAccel){
	LUA_INTARG(1); //entity id
	PushFloatArray(L, entities[arg1].body.a.d, 3);
	return 1;
}
LUA_EXPORT(entity_setAccel){
	LUA_INTARG(1); //entity id
	vec3 temp;
	popVec3fromArray(L, &temp);
	entities[arg1].body.a = temp;
	return 0;
}
LUA_EXPORT(entity_setShape){
	LUA_INTARG(1); //entity id
	float temp[7];
	popFloatArray(L, temp, 7);
	entities[arg1].body.shape.c.d[0] = temp[0];
	entities[arg1].body.shape.c.d[1] = temp[1];
	entities[arg1].body.shape.c.d[2] = temp[2];
	entities[arg1].body.shape.c.d[3] = temp[3];
	entities[arg1].body.shape.e.d[0] = temp[4];
	entities[arg1].body.shape.e.d[1] = temp[5];
	entities[arg1].body.shape.e.d[2] = temp[6];	
	return 0;
}
LUA_EXPORT(entity_getShape){
	LUA_INTARG(1); //entity id
	float temp[7];
	temp[0] = (entities[arg1].body.shape.c.d[0]);
	temp[1] = (entities[arg1].body.shape.c.d[1]);
	temp[2] = (entities[arg1].body.shape.c.d[2]);
	temp[3] = (entities[arg1].body.shape.c.d[3]);
	temp[4] = (entities[arg1].body.shape.e.d[0]);
	temp[5] = (entities[arg1].body.shape.e.d[1]);
	temp[6] = (entities[arg1].body.shape.e.d[2]);
	PushFloatArray(L, temp, 7);
	return 1;
}
LUA_EXPORT(entity_setBounciness){
	LUA_INTARG(1); //entity id
	LUA_FLOATARG(2); //mass
	entities[arg1].body.bounciness = arg2;
	return 0;
}
LUA_EXPORT(entity_setFriction){
	LUA_INTARG(1); //entity id
	LUA_FLOATARG(2); //mass
	entities[arg1].body.friction = arg2;
	return 0;
}
LUA_EXPORT(entity_setAirFriction){
	LUA_INTARG(1); //entity id
	LUA_FLOATARG(2); //mass
	entities[arg1].body.airfriction = arg2;
	return 0;
}
LUA_EXPORT(entity_getMass){
	LUA_INTARG(1); //entity id
	lua_pushinteger(L, entities[arg1].body.mass);
	return 1;
}
LUA_EXPORT(entity_getBounciness){
	LUA_INTARG(1); //entity id
	lua_pushinteger(L, entities[arg1].body.bounciness);
	return 1;
}
LUA_EXPORT(entity_getFriction){
	LUA_INTARG(1); //entity id
	lua_pushinteger(L, entities[arg1].body.friction);
	return 1;
}
LUA_EXPORT(entity_getAirFriction){
	LUA_INTARG(1); //entity id
	lua_pushinteger(L, entities[arg1].body.airfriction);
	return 1;
}
LUA_EXPORT(entity_getDL){
	LUA_INTARG(1); //entity id
	lua_pushinteger(L, entities[arg1].dl);
	return 1;
}
LUA_EXPORT(setGravity){
	vec3 temp;
	popVec3fromArray(L, &temp);
	entity_world.world.g = temp;
	return 0;
}
LUA_EXPORT(setMS){
	LUA_FLOATARG(1);
	entity_world.world.ms = arg1;
	return 0;
}
LUA_EXPORT(getMS){
	(void)L;
	lua_pushnumber(L, entity_world.world.ms);
	return 1;
}
LUA_EXPORT(getGravity){
	(void)L;
	PushFloatArray(L, entity_world.world.g.d, 3);
	return 1;
}
LUA_EXPORT(engine_abort){
(void)L;
	isRunning = 0;
	return 0;
}
LUA_EXPORT(get_maxEnts){
(void)L;
	lua_pushinteger(L, entity_world.max_ents);
	return 1;
}
LUA_EXPORT(get_nEnts){
(void)L;
	lua_pushinteger(L, entity_world.n_ents);
	return 1;
}
LUA_EXPORT(removeEntity){
	LUA_INTARG(1);
	ChadWorld_RemoveEntityByPointer(&entity_world, entities + arg1);
	return 0;
}
LUA_EXPORT(addEntity){
	LUA_INTARG(1);
	ChadWorld_AddEntity(&entity_world, entities + arg1);
	return 0;
}
LUA_EXPORT(lMus){
	LUA_INTARG(1);
	LUA_STRINGARG(2);
	if(tracks[arg1]) Mix_FreeMusic(tracks[arg1]);
	tracks[arg1] = lmus(arg2);
	return 0;
}

LUA_EXPORT(lWav){
	LUA_INTARG(1);
	LUA_STRINGARG(2);
	if(samps[arg1]) {Mix_FreeChunk(samps[arg1]);}
	samps[arg1] = lwav(arg2);
	return 0;
}

LUA_EXPORT(aHalt){
	LUA_INTARG(1);
	aHalt(arg1);
	return 0;
}

LUA_EXPORT(mHalt){
	(void)L;
	mhalt();
	return 0;
}

LUA_EXPORT(aPos){
	LUA_INTARG(1);
	LUA_INTARG(2);
	LUA_INTARG(3);
	aPos(arg1, arg2, arg3);
	return 0;
}

LUA_EXPORT(aPlay){
	LUA_INTARG(1);
	LUA_INTARG(2);
	if(samps[arg1]){
		lua_pushinteger(L, aplay(samps[arg1], arg2));
		return 1;
	}
	puts("Cannot play unloaded sample.");
	lua_pushinteger(L, -1);
	return 1;
}

LUA_EXPORT(applyCamera3D){
	(void)L;
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(camproj.d);
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(camview.d);
	return 0;
}
LUA_EXPORT(applyCamera2D){
	(void)L;
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(camview.d);
	return 0;
}



LUA_EXPORT(mPlay){
	LUA_INTARG(1);
	LUA_INTARG(2);
	LUA_INTARG(3);
	if(tracks[arg1]){
		lua_pushinteger(L, mplay(tracks[arg1], arg2, arg3));
		return 1;
	}
	puts("Cannot play unloaded track.");
	lua_pushinteger(L, -1);
	return 1;
}

LUA_EXPORT(dWav){
	LUA_INTARG(1);
	if(samps[arg1]) Mix_FreeChunk(samps[arg1]);
	samps[arg1] = NULL;
	return 0;
}

LUA_EXPORT(dMus){
	LUA_INTARG(1);
	if(tracks[arg1]) Mix_FreeMusic(tracks[arg1]);
	tracks[arg1] = NULL;
	return 0;
}

LUA_EXPORT(drawBox){
	LUA_FLOATARG(1);
	LUA_FLOATARG(2);
	LUA_FLOATARG(3);
	LUA_FLOATARG(4);
	drawBox(arg1, arg2, arg3, arg4);
	return 0;
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
	int r = omg_box(arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8);
	lua_pushinteger(L, r);
	return 1;
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
	int r = omg_textbox(arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9);
	
	lua_pushinteger(L, r);
	return 1;
}
LUA_EXPORT(build_camview){
	vec3 temp1;
	vec3 temp2;
	vec3 temp3;
	popVec3fromArray(L, &temp3);lua_pop(L, 1);
	popVec3fromArray(L, &temp2);lua_pop(L, 1);
	popVec3fromArray(L, &temp1);lua_pop(L, 1);
	camview = lookAt(
						temp1, 
						temp2,
						temp3);
	PushFloatArray(L, camview.d, 16);
	return 1;
}
LUA_EXPORT(lookAt){
	mat4 retval;
	vec3 temp1;
	vec3 temp2;
	vec3 temp3;
	popVec3fromArray(L, &temp3);lua_pop(L, 1);
	popVec3fromArray(L, &temp2);lua_pop(L, 1);
	popVec3fromArray(L, &temp1);lua_pop(L, 1);
	retval = lookAt(
						temp1, 
						temp2,
						temp3);
	PushFloatArray(L, retval.d, 16);
	return 1;
}
LUA_EXPORT(build_camview2D){
	vec3 temp;
	popVec3fromArray(L, &temp);
	camview = translate(temp);
	return 0;
}
LUA_EXPORT(translate){
	vec3 temp; mat4 res;
	popVec3fromArray(L, &temp);
	res = translate(temp);
	PushFloatArray(L, res.d, 16);
	return 1;
}
LUA_EXPORT(get_camview){
	PushFloatArray(L, camview.d, 16);
	return 1;
}
int lua_loadTexture(lua_State* L){
	const char* texturename = lua_tostring(L, 1);
	{
		int sw, sh, sc; GLint retval;
		uchar* source_data = stbi_load(texturename, &sw, &sh, &sc, 3);
		if(!source_data){
			lua_pushinteger(L, -1);
			printf("\nERROR!!! Cannot load '%s'!!!\n", texturename);
			lua_pushinteger(L, -1);
			return 1;
		}
		retval = loadRGBTexture(source_data, sw, sh);
		free(source_data);
		lua_pushinteger(L, retval);
	}
	return 1;
}
int lua_resetProj(lua_State* L){
	(void)L;
	camproj = identitymat4();
	PushFloatArray(L, camproj.d, 16);
	return 1;
}
int lua_setPerspective(lua_State* L){
	LUA_FLOATARG(1);
	LUA_FLOATARG(2);
	LUA_FLOATARG(3);
	LUA_FLOATARG(4);
	camproj = perspective(arg1,arg2,arg3,arg4);
	return 0;
}
LUA_EXPORT(perspective){
	mat4 retval;
	LUA_FLOATARG(1);
	LUA_FLOATARG(2);
	LUA_FLOATARG(3);
	LUA_FLOATARG(4);
	retval = perspective(arg1,arg2,arg3,arg4);
	PushFloatArray(L, retval.d, 16);
	return 1;
}

int lua_bindTexture(lua_State* L){
	GLint texture_id = lua_tointeger(L, 1);
	glBindTexture(GL_TEXTURE_2D, texture_id);
	return 0;
}

int lua_deleteTexture(lua_State* L){
	GLuint texture_id = lua_tointeger(L, 1);
	glDeleteTextures(1, &texture_id);
	return 0;
}

int lua_deleteList(lua_State* L){
	GLint texture_id = lua_tointeger(L, 1);
	glDeleteLists(texture_id, 1);
	return 0;
}

int lua_callList(lua_State* L){
	GLint texture_id = lua_tointeger(L, 1);
	glCallList(texture_id);
	return 0;
}
int lua_setTexturingEnabled(lua_State* L){
	GLint arg = lua_tointeger(L, 1);
	if(arg) glEnable(GL_TEXTURE_2D);
	else glDisable(GL_TEXTURE_2D);
	return 0;
}
int lua_set2D(lua_State* L){
	GLint arg = lua_tointeger(L, 1);
	if(arg) entity_world.world.is_2d = 1;
	else entity_world.world.is_2d = 0;
	return 0;
}
int lua_buildSpriteDL(lua_State* L){
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
		drawBox(-arg1, -arg2,
				arg1 * 2.0, arg2 * 2.0); //centered.
		glDisable(GL_TEXTURE_2D);
	glEndList();
	lua_pushinteger(L, display_list);
	return 1;
}


int lua_buildRectangleDL(lua_State* L){
	LUA_FLOATARG(1); //width
	arg1/=2.0;
	LUA_FLOATARG(2); //height
	arg2/=2.0;
	LUA_FLOATARG(3); //R
	LUA_FLOATARG(4); //G
	LUA_FLOATARG(5); //B
	GLuint display_list = glGenLists(1);
	glNewList(display_list, GL_COMPILE);
		glDisable(GL_TEXTURE_2D);
		glColor3f(arg3,arg4,arg5);
		drawBox(-arg1, -arg2,
				arg1 * 2.0, arg2 * 2.0); //centered.
		glDisable(GL_TEXTURE_2D);
	glEndList();
	lua_pushinteger(L, display_list);
	return 1;
}


int lua_buildModelDL(lua_State* L){
	const char* objname = lua_tostring(L, 1);
	GLint texture_id = lua_tointeger(L, 2);
	{
		objraw omodel;
		model m = initmodel();
		omodel = tobj_load(objname);
		if (!omodel.positions){ //error.
			printf("\nERROR!!! Loading model '%s' results in ZERO POSITIONS!!!\n", objname);
			lua_pushinteger(L, -1);
			return 0;
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
			glDisable(GL_TEXTURE_2D);
			glEndList();
			freemodel(&m);
			freeobjraw(&omodel);
			lua_pushinteger(L, list);
		}
	}
	return 1;
}

void createLuaBindings(){
	lua_register(L_STATE, "loadTexture", lua_loadTexture);
	lua_register(L_STATE, "bindTexture", lua_bindTexture);
	lua_register(L_STATE, "callList", lua_callList);
	lua_register(L_STATE, "deleteList", lua_deleteList);
	lua_register(L_STATE, "deleteTexture", lua_deleteTexture);
	lua_register(L_STATE, "set2D", lua_set2D);
	lua_register(L_STATE, "setPerspective", lua_setPerspective);
	LUA_IMPORT(drawBox);
	LUA_IMPORT(buildSpriteDL);
	LUA_IMPORT(buildRectangleDL);
	LUA_IMPORT(buildModelDL);
	LUA_IMPORT(omg_box);
	LUA_IMPORT(omg_textbox);
	LUA_IMPORT(setTexturingEnabled);
	LUA_IMPORT(resetProj);
	LUA_IMPORT(removeEntity);
	LUA_IMPORT(addEntity);
	LUA_IMPORT(lMus);
	LUA_IMPORT(lWav);
	LUA_IMPORT(dMus);
	LUA_IMPORT(dWav);
	LUA_IMPORT(aPlay);
	LUA_IMPORT(mPlay);
	LUA_IMPORT(aHalt);
	LUA_IMPORT(mHalt);
	LUA_IMPORT(aPos);
	LUA_IMPORT(engine_abort);
	LUA_IMPORT(entity_setDL);
	LUA_IMPORT(entity_getDL);
	LUA_IMPORT(entity_setLocalT);
	LUA_IMPORT(entity_getMass);
	LUA_IMPORT(entity_setMass);
	LUA_IMPORT(entity_getFriction);
	LUA_IMPORT(entity_setFriction);
	LUA_IMPORT(entity_setAirFriction);
	LUA_IMPORT(entity_getAirFriction);
	LUA_IMPORT(entity_getVelocity);
	LUA_IMPORT(entity_setVelocity);
	LUA_IMPORT(entity_getAccel);
	LUA_IMPORT(entity_setAccel);
	LUA_IMPORT(entity_getShape);
	LUA_IMPORT(entity_setShape);
	LUA_IMPORT(entity_getBounciness);
	LUA_IMPORT(entity_setBounciness);
	LUA_IMPORT(get_maxEnts);
	LUA_IMPORT(get_nEnts);
	LUA_IMPORT(setGravity);
	LUA_IMPORT(getGravity);
	LUA_IMPORT(setMS);
	LUA_IMPORT(getMS);
	LUA_IMPORT(ld_mat4);
	LUA_IMPORT(st_mat4);
	LUA_IMPORT(ld_vec4);
	LUA_IMPORT(st_vec4);
	LUA_IMPORT(ld_vec3);
	LUA_IMPORT(st_vec3);
	LUA_IMPORT(dotv3);
	LUA_IMPORT(scalev3);
	LUA_IMPORT(dotv4);
	LUA_IMPORT(scalev4);
	LUA_IMPORT(mov_v3);
	LUA_IMPORT(mov_v4);
	LUA_IMPORT(mov_mat4);
	LUA_IMPORT(multm4);
	LUA_IMPORT(addv3);
	LUA_IMPORT(subv3);
	LUA_IMPORT(addv4);
	LUA_IMPORT(subv4);
	LUA_IMPORT(build_camview);
	LUA_IMPORT(build_camview2D);
	LUA_IMPORT(get_camview);
	LUA_IMPORT(applyCamera3D);
	LUA_IMPORT(applyCamera2D);
	LUA_IMPORT(glRotate3f);
	LUA_IMPORT(glTranslatef);
	LUA_IMPORT(glPushMatrix);
	LUA_IMPORT(glPopMatrix);
	LUA_IMPORT(glMultMatrixf);
	LUA_IMPORT(lookAt);
	LUA_IMPORT(setEnableDepthTest);
	LUA_IMPORT(setEnableLighting);
	LUA_IMPORT(setEnableColorMaterial);
	LUA_IMPORT(setColorMaterialMode);
	LUA_IMPORT(setEnableLight);
	LUA_IMPORT(setLightProps);
	LUA_IMPORT(setCullingMode);
	LUA_IMPORT(stepChadWorld);
	LUA_IMPORT(renderChadWorld);
	LUA_IMPORT(glScalef);
	LUA_IMPORT(perspective);
	LUA_IMPORT(translate);
	LUA_IMPORT(glClearColor);
	LUA_IMPORT(glClear);
}


void setGlobals(){
	int button1 = sixbuttons[0];
	int button2 = sixbuttons[1];
	int button3 = sixbuttons[2];
	int button4 = sixbuttons[3];
	int button5 = sixbuttons[4];
	int button6 = sixbuttons[5];
	int buttonup = dirbstates[0];
	int buttondown = dirbstates[1];
	int buttonleft = dirbstates[2];
	int buttonright = dirbstates[3];
	int omg_udlr_u = omg_udlr[0];
	int omg_udlr_d = omg_udlr[1];
	int omg_udlr_l = omg_udlr[2];
	int omg_udlr_r = omg_udlr[3];
	double mousex = mousepos[0];
	double mousey = mousepos[1];
	LUA_INTGLOBSET(button1);
	LUA_INTGLOBSET(button2);
	LUA_INTGLOBSET(button3);
	LUA_INTGLOBSET(button4);
	LUA_INTGLOBSET(button5);
	LUA_INTGLOBSET(button6);
	LUA_INTGLOBSET(buttonup);
	LUA_INTGLOBSET(buttondown);
	LUA_INTGLOBSET(buttonleft);
	LUA_INTGLOBSET(buttonright);
	LUA_INTGLOBSET(omg_cb);
	LUA_INTGLOBSET(omg_udlr_u);
	LUA_INTGLOBSET(omg_udlr_d);
	LUA_INTGLOBSET(omg_udlr_l);
	LUA_INTGLOBSET(omg_udlr_r);
	LUA_INTGLOBSET(mb2);
	LUA_INTGLOBSET(mb);
	LUA_INTGLOBSET(using_cursorkeys);
	LUA_INTGLOBSET(isRunning);
	LUA_FLOATGLOBSET(mousex);
	LUA_FLOATGLOBSET(mousey);
	LUA_FLOATGLOBSET(tpassed);
}
void draw_menu() {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//TODO: invoke lua for draw menu.
	/*
	if (omg_textbox(0.01, 0.6, "\nQuit\n", 24, 1, 0.4, 0.2, 0xFFFFFF, 0) && omg_cb == 2) {
		puts("Quitting...");
		isRunning = 0;
	}
	*/
	setGlobals();
	luaL_dostring(L_STATE, "drawMenu()");
	//drawMouse(); //TODO 
}

void draw_gameplay(){
	//TODO: register global variables.
setGlobals();
	luaL_dostring(L_STATE, "draw()");
}
void draw(){
	lua_pushinteger(L_STATE, winSizeX); lua_setglobal(L_STATE, "winSizeX");
	if(is_in_menu) draw_menu();
	else draw_gameplay();
}
void cleanup(){
	luaL_dostring(L_STATE, "cleanup()");
}

void initScene() {
	glClearColor(0.0, 0.0, 0.3, 0.0);
	glViewport(0, 0, winSizeX, winSizeY);
	SDL_ShowCursor(SDL_DISABLE);
	SDL_WM_SetCaption("Video Game", 0);
	{
		camproj = identitymat4();
		camview = identitymat4();
	}
	entity_world.ents = calloc(			1, MAX_ENTITIES * sizeof(void*));
	entity_world.world.bodies = calloc(	1, MAX_ENTITIES * sizeof(void*));
	
	entity_world.n_ents = 0;
	entity_world.max_ents = MAX_ENTITIES;
	//TODO: invoke lua for initscene.
	lua_pushinteger(L_STATE, winSizeX); lua_setglobal(L_STATE, "winSizeX");
	lua_pushinteger(L_STATE, winSizeY); lua_setglobal(L_STATE, "winSizeY");
	luaL_dostring(L_STATE, "init()");
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
		case SDLK_z: sixbuttons[0] = 1; break;
		case SDLK_x: sixbuttons[1] = 1; break;
		case SDLK_c: sixbuttons[2] = 1; break;
		case SDLK_a: sixbuttons[3] = 1; break;
		case SDLK_s: sixbuttons[4] = 1; break;
		case SDLK_d: sixbuttons[5] = 1; break;
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
		case SDLK_z: sixbuttons[0] = 0; break;
		case SDLK_x: sixbuttons[1] = 0; break;
		case SDLK_c: sixbuttons[2] = 0; break;
		case SDLK_a: sixbuttons[3] = 0; break;
		case SDLK_s: sixbuttons[4] = 0; break;
		case SDLK_d: sixbuttons[5] = 0; break;
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
