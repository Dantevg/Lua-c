#include <SDL.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "util.c"

struct Timer {
	int id;     // The function id in the lua registry
	int delay;  // The delay in ms
	int repeat; // 1 = repeat, 0 = don't repeat
};

struct Callback {
	int fn;
	int data;
};

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

int loop(unsigned int dt){
	// Events
	SDL_Event event;
	while(SDL_PollEvent(&event)){
		if(event.type == SDL_QUIT){
			return 1;
		}else if(event.type == SDL_USEREVENT && event.user.code == 1){
			// Got callback event, call Lua callback
			struct Timer *timer = (struct Timer*)event.user.data1;
			lua_rawgeti(L, LUA_REGISTRYINDEX, timer->id);
			lua_pushinteger(L, timer->delay);
			if(lua_pcall(L, 1, 0, 0) != LUA_OK){
				printf("%s\n", lua_tostring(L, -1));
			}
			
			// struct Timer *timer = (struct Timer*)event.user.data1;
			// lua_rawgeti(L, LUA_REGISTRYINDEX, timer->id);
			// lua_pushinteger(L, timer->delay);
			// if(lua_pcall(L, 1, 0, 0) != LUA_OK){
			// 	printf("%s\n", lua_tostring(L, -1));
			// }
		}else if(event.type == SDL_TEXTINPUT){
			lua_getfield(L, LUA_REGISTRYINDEX, "SDL_TEXTINPUT"); // stack: {SDL_TEXTINPUT, ...}
			int n = luaL_len(L, -1);
			
			// Loop through all registered callbacks
			for(int i = 0; i < n-1; i++ ){
				printf("SDL_TEXTINPUT\n");
				lua_rawgeti(L, -1, i); // stack: {SDL_TEXTINPUT[i], SDL_TEXTINPUT, ...}
				void *ptr = lua_touserdata(L, -1);
				struct Callback *callback = (struct Callback*)ptr;
				lua_pop(L, 1); // stack: {SDL_TEXTINPUT, ...}
				lua_rawgeti(L, LUA_REGISTRYINDEX, callback->fn); // stack: {fn, SDL_TEXTINPUT, ...}
				lua_rawgeti(L, LUA_REGISTRYINDEX, callback->data); // stack: {data, fn, SDL_TEXTINPUT, ...}
				if(lua_pcall(L, 1, 0, 0) != LUA_OK){ // stack: {(err?), SDL_TEXTINPUT, ...}
					printf("%s\n", lua_tostring(L, -1));
					lua_pop(L, 1); // stack: {SDL_TEXTINPUT, ...}
				}
			}
			lua_pop(L, 1); // stack: {...}
		}
	}
	
	return 0;
}

int main(int argc, char *argv[]){
	// Init SDL
	if(SDL_Init(SDL_INIT_VIDEO) < 0){
		printf("Could not initialize SDL: %s\n", SDL_GetError());
		return -1;
	}
	
	// Make sure the program closes as soon as possible
	// to prevent infinite loops or complex rendering from preventing closing
	SDL_AddEventWatch(quit_callback, NULL);
	
	// Init Lua
	L = luaL_newstate();
	luaL_openlibs(L); // Open standard libraries (math, string, table, ...)
	
	// Set cpath
	luaL_dostring(L, "package.cpath = package.cpath..';./bin/?.so'");
	
	// Register event
	lua_newtable(L);
	lua_setfield(L, LUA_REGISTRYINDEX, "SDL_TEXTINPUT");
	
	// Load main file
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
	
	// Main loop
	t0 = SDL_GetTicks();
	unsigned int t_prev = t0;
	int quit = 0;
	while(!quit){
		unsigned int t_new = SDL_GetTicks();
		quit = loop(t_new - t_prev);
		t_prev = t_new;
	}
	
	// Exit
	SDL_Quit();
	lua_close(L);
	
	return 0;
}