#include <time.h>   // for clock_gettime, compile with -std=gnu99

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "MoonBox.h"
#include "event.h"

int mb_error_handler(lua_State *L){
	luaL_traceback(L, L, lua_tostring(L, -1), 2);
	fprintf(stderr, "%s\n", lua_tostring(L, -1));
	lua_pop(L, 1);
	return 1;
}

int mb_os_clock(lua_State *L){
	lua_getfield(L, LUA_REGISTRYINDEX, "mb_clock_base");
	struct timespec *base = lua_touserdata(L, -1);
	lua_pop(L, 1);
	struct timespec t;
	clock_gettime(CLOCK_MONOTONIC, &t);
	time_t diff_sec = t.tv_sec - base->tv_sec;
	time_t diff_nsec = (t.tv_nsec - base->tv_nsec);
	lua_pushnumber(L, diff_sec + (double)diff_nsec*1e-9);
	return 1;
}

// Returns the time difference in microseconds
static time_t timediff(struct timespec *base){
	struct timespec t;
	clock_gettime(CLOCK_MONOTONIC, &t);
	time_t diff_sec = t.tv_sec - base->tv_sec;
	time_t diff_nsec = (t.tv_nsec - base->tv_nsec);
	return diff_sec*1e6 + diff_nsec*0.001;
}

int mb_os_sleep(lua_State *L){
	time_t microseconds = lua_tonumber(L, 1) * 1e6;
	struct timespec base;
	clock_gettime(CLOCK_MONOTONIC, &base);
	while(timediff(&base) < microseconds){
		event_loop(L);
	}
	return 0;
}

lua_State *mb_init(){
	lua_State *L = luaL_newstate();
	luaL_openlibs(L); // Open standard libraries (math, string, table, ...)
	
	/* Set cpath and path */
	if(luaL_dostring(L, "package.cpath = package.cpath..';" BASE_PATH "bin/?." SO_EXT "'")){
		fprintf(stderr, "[C] Could not set package.cpath:\n%s\n", lua_tostring(L, -1));
	}
	if(luaL_dostring(L, "package.path = package.path..';" BASE_PATH "res/lib/?.lua;" BASE_PATH "res/lib/?/init.lua'")){
		fprintf(stderr, "[C] Could not set package.path:\n%s\n", lua_tostring(L, -1));
	}
	
	/* Push MoonBox version */
	lua_pushstring(L, "MoonBox " VERSION);
	lua_setglobal(L, "_MB_VERSION");
	
	/* Push new os.clock and os.sleep */
	lua_getglobal(L, "os");
	lua_pushcfunction(L, mb_os_clock);
	lua_setfield(L, -2, "clock");
	lua_pushcfunction(L, mb_os_sleep);
	lua_setfield(L, -2, "sleep");
	lua_pop(L, 1);
	
	/* Push Lua error handler */
	lua_pushcfunction(L, mb_error_handler);
	lua_pushvalue(L, 1);
	lua_setfield(L, LUA_REGISTRYINDEX, "mb_error_handler");
	
	/* Set clock start time */
	struct timespec *clock_base = lua_newuserdata(L, sizeof(struct timespec));
	lua_setfield(L, LUA_REGISTRYINDEX, "mb_clock_base");
	clock_gettime(CLOCK_MONOTONIC, clock_base);
	
	/* Run init file */
	if(luaL_loadfile(L, BASE_PATH "res/init.lua") == LUA_OK){
		if(lua_pcall(L, 0, 0, 1) != LUA_OK) return NULL;
	}else{
		fprintf(stderr, "%s\n", lua_tostring(L, -1));
		return NULL;
	}
	
	return L;
}

int mb_load(lua_State *L, const char *file){
	if(luaL_loadfile(L, file) == LUA_OK){
		return 1;
	}else{
		fprintf(stderr, "[C] Could not load Lua code: %s\n", lua_tostring(L, -1));
		return 0;
	}
}

void mb_run(lua_State *L, int n_args, int loop){
	lua_rotate(L, lua_gettop(L)-n_args, 1);
	if(lua_pcall(L, n_args, 1, 1) == LUA_OK){
		// Immediately stop execution when main chunk returns false
		if(lua_isboolean(L, -1) && lua_toboolean(L, -1) == 0){
			lua_close(L);
			return;
		}
		lua_pop(L, 1);
	}else{
		// Error message was already printed by mb_error_handler
		return;
	}
	
	/* Main loop */
	int quit = !loop;
	while(!quit){
		quit = event_loop(L);
	}
}

void mb_main(lua_State *L, const char *file, int n_args){
	/* Load file */
	if(!mb_load(L, file)){
		lua_close(L);
		return;
	}
	
	/* Call chunk */
	mb_run(L, n_args, 1);
}