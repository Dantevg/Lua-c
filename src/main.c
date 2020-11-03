#include <unistd.h> // For chdir

#include <SDL2/SDL.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "main.h"

#define VERSION "0.1.0"

#if defined(_WIN32)
	#define SO_EXT "dll"
#else
	#define SO_EXT "so"
#endif

// Callback to quit program as soon as possible upon user request
int quit_callback(void *userdata, SDL_Event *event){
	if(event->type == SDL_QUIT){
		lua_State *L = (lua_State *)userdata;
		SDL_Quit();
		lua_close(L);
		exit(0);
	}
	return 0;
}

int loop(lua_State *L){
	/* Events */
	SDL_Event event;
	while(SDL_PollEvent(&event)){
		if(event.type == SDL_QUIT){
			return 1;
		}
		if(event_dispatch_ptr == NULL){
			lua_getfield(L, LUA_REGISTRYINDEX, "event_dispatch");
			if(lua_islightuserdata(L, -1)){
				event_dispatch_ptr = lua_touserdata(L, -1);
			}
			lua_pop(L, 1);
		}
		if(event_dispatch_ptr != NULL){
			(*event_dispatch_ptr)(L, &event);
		}
	}
	
	return 0;
}

int main(int argc, char *argv[]){
	char *file = "res/main.lua";
	
	/* Parse command-line arguments */
	for(int i = 1; i < argc; i++){
		if(strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0){
			/* Print version and return */
			printf("%s\n", VERSION);
			return 0;
		}else if(strcmp(argv[i], "-") == 0){
			/* Execute stdin */
			file = NULL;
			break;
		}else{
			/* Execute file */
			file = argv[i];
		}
	}
	
	/* Init SDL */
	if(SDL_Init(0) < 0){
		fprintf(stderr, "[C] Could not initialize SDL: %s\n", SDL_GetError());
		return -1;
	}
	
	/* Init Lua */
	lua_State *L = luaL_newstate();
	luaL_openlibs(L); // Open standard libraries (math, string, table, ...)
	
	// Make sure the program closes as soon as possible
	// to prevent infinite loops or complex rendering from preventing closing
	SDL_AddEventWatch(quit_callback, L);
	
	/* Set cpath and path */
	if(luaL_dostring(L, "package.cpath = package.cpath..';../bin/?." SO_EXT "'")){
		fprintf(stderr, "[C] Could not set package.cpath: %s\n", lua_tostring(L, -1));
	}
	// if(luaL_dostring(L, "package.path = package.path..';../res/?.lua'")){
	// 	fprintf(stderr, "[C] Could not set package.path: %s\n", lua_tostring(L, -1));
	// }
	
	/* Register callbacks table */
	lua_newtable(L); // stack: {tbl}
	lua_pushinteger(L, 0); // stack: {0, tbl}
	lua_setfield(L, -2, "n"); // stack: {tbl}
	lua_setfield(L, LUA_REGISTRYINDEX, "callbacks"); // stack: {}
	
	/* Load main file */
	if(luaL_loadfile(L, file) == LUA_OK){
		chdir("res");
		if(lua_pcall(L, 0, 1, 0) == LUA_OK){
			// Immediately stop execution when main chunk returns false
			if(lua_isboolean(L, -1) && lua_toboolean(L, -1) == 0){
				return 0;
			}
		}else{
			fprintf(stderr, "%s\n", lua_tostring(L, -1));
			return -1;
		}
	}else{
		fprintf(stderr, "[C] Could not load Lua code: %s\n", lua_tostring(L, -1));
		return -1;
	}
	
	/* Main loop */
	int quit = 0;
	while(!quit){
		quit = loop(L);
	}
	
	/* Exit */
	SDL_Quit();
	lua_close(L);
	
	return 0;
}