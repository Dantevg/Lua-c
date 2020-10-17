#include <SDL.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "main.h"
#include "util.c"

lua_State *L;
unsigned int t0;

// Callback to quit program as soon as possible upon user request
int quit_callback(void *userdata, SDL_Event *event){
	if(event->type == SDL_QUIT){
		SDL_Quit();
		lua_close(L);
		exit(0);
	}
	return 0;
}

void dispatch_callbacks(char *eventname, int args){
	// stack: {eventdata, ...}
	/* Get callbacks table */
	lua_getfield(L, LUA_REGISTRYINDEX, "callbacks"); // stack: {callbacks, eventdata, ...}
	lua_getfield(L, -1, "n"); // stack: {n, callbacks, eventdata, ...}
	int n = lua_tointeger(L, -1);
	lua_pop(L, 1); // stack: {callbacks, eventdata, ...}
	
	/* Loop through all registered callbacks */
	for(int i = 1; i <= n; i++){
		/* Get callback struct from userdata */
		lua_rawgeti(L, -1, i); // stack: {callbacks[i], callbacks, eventdata, ...}
		void *ptr = lua_touserdata(L, -1);
		if(ptr == NULL){ // No callback at this position
			lua_pop(L, 1); // stack: {callbacks, eventdata, ...}
			continue;
		}
		Callback *callback = (Callback*)ptr;
		lua_pop(L, 1); // stack: {callbacks, eventdata, ...}
		
		if(strcmp(callback->event, eventname) == 0){
			/* Callback is for current event */
			lua_rawgeti(L, LUA_REGISTRYINDEX, callback->fn); // stack: {fn, callbacks, eventdata, ...}
			lua_pushstring(L, eventname);
			for(int i = 0; i < args; i++){ // Push all arguments on top of stack
				lua_pushvalue(L, -3-args); // stack: {eventdata, fn, callbacks, eventdata, ...}
			}
			if(lua_pcall(L, args+1, 0, 0) != LUA_OK){ // stack: {(err?), callbacks, eventdata, ...}
				printf("%s\n", lua_tostring(L, -1));
				lua_pop(L, 1); // stack: {callbacks, eventdata, ...}
			}
			// TODO: let callback return false to disable it,
			// or pass callback ID (ideally, both)
		}
	}
	lua_pop(L, 2); // stack: {...}
}

int loop(unsigned int dt){
	/* Events */
	SDL_Event event;
	while(SDL_PollEvent(&event)){
		if(event.type == SDL_QUIT){
			return 1;
		}else if(event.type == SDL_USEREVENT && event.user.code == 1){
			/* Got callback event, call Lua callback */
			Callback *callback = event.user.data1;
			lua_rawgeti(L, LUA_REGISTRYINDEX, callback->fn); // stack: {fn, ...}
			lua_pushstring(L, "timer"); // stack: {"timer", fn, ...}
			lua_pushinteger(L, ((Timer*)callback->data)->delay); // stack: {delay, "timer", fn, ...}
			
			if(lua_pcall(L, 2, 0, 0) != LUA_OK){ // stack: {(err?), ...}
				printf("%s\n", lua_tostring(L, -1));
				lua_pop(L, 1); // stack: {...}
			}
		}else if(event.type == SDL_KEYDOWN){
			lua_pushstring(L, SDL_GetKeyName(event.key.keysym.sym));
			dispatch_callbacks("kb.down", 1);
		}else if(event.type == SDL_KEYUP){
			lua_pushstring(L, SDL_GetKeyName(event.key.keysym.sym));
			dispatch_callbacks("kb.up", 1);
		}else if(event.type == SDL_TEXTINPUT){
			lua_pushstring(L, event.text.text);
			dispatch_callbacks("kb.input", 1);
		}else if(event.type == SDL_MOUSEMOTION){
			lua_pushinteger(L, event.motion.x);
			lua_pushinteger(L, event.motion.y);
			dispatch_callbacks("mouse.move", 2);
		}else if(event.type == SDL_MOUSEBUTTONDOWN){
			lua_pushinteger(L, event.button.button);
			lua_pushinteger(L, event.button.x);
			lua_pushinteger(L, event.button.y);
			lua_pushboolean(L, event.button.clicks-1);
			dispatch_callbacks("mouse.down", 4);
		}else if(event.type == SDL_MOUSEBUTTONUP){
			lua_pushinteger(L, event.button.button);
			lua_pushinteger(L, event.button.x);
			lua_pushinteger(L, event.button.y);
			lua_pushboolean(L, event.button.clicks-1);
			dispatch_callbacks("mouse.up", 4);
		}else if(event.type == SDL_MOUSEWHEEL){
			lua_pushinteger(L, event.wheel.x);
			lua_pushinteger(L, event.wheel.y);
			lua_pushboolean(L, event.wheel.direction);
			dispatch_callbacks("mouse.scroll", 3);
		}else if(event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED){
			lua_pushinteger(L, event.window.data1);
			lua_pushinteger(L, event.window.data2);
			dispatch_callbacks("screen.resize", 2);
		}
	}
	
	return 0;
}

int main(int argc, char *argv[]){
	/* Init SDL */
	if(SDL_Init(SDL_INIT_VIDEO) < 0){
		printf("Could not initialize SDL: %s\n", SDL_GetError());
		return -1;
	}
	
	// Make sure the program closes as soon as possible
	// to prevent infinite loops or complex rendering from preventing closing
	SDL_AddEventWatch(quit_callback, NULL);
	
	/* Init Lua */
	L = luaL_newstate();
	luaL_openlibs(L); // Open standard libraries (math, string, table, ...)
	
	/* Set cpath and path */
	luaL_dostring(L, "package.cpath = package.cpath..';./bin/?.so'");
	luaL_dostring(L, "package.path = package.path..';res/?.lua'");
	
	/* Register callbacks table */
	lua_newtable(L); // stack: {tbl}
	lua_pushinteger(L, 0); // stack: {0, tbl}
	lua_setfield(L, -2, "n"); // stack: {tbl}
	lua_setfield(L, LUA_REGISTRYINDEX, "callbacks"); // stack: {}
	
	/* Load main file */
	if(luaL_loadfile(L, "res/main.lua") == LUA_OK){
		if(lua_pcall(L, 0, 0, 0) == LUA_OK){
			printf("[C] Code executed successfully\n");
		}else{
			printf("[C] Error in Lua code: %s\n", lua_tostring(L, -1));
			return -1;
		}
	}else{
		printf("[C] Could not load Lua code: %s\n", lua_tostring(L, -1));
		return -1;
	}
	
	/* Main loop */
	t0 = SDL_GetTicks();
	unsigned int t_prev = t0;
	int quit = 0;
	while(!quit){
		unsigned int t_new = SDL_GetTicks();
		quit = loop(t_new - t_prev);
		t_prev = t_new;
	}
	
	/* Exit */
	SDL_Quit();
	lua_close(L);
	
	return 0;
}