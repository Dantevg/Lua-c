#include <SDL2/SDL.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "util.h"
#include "thread.h"

/* C library definitions */

// Gets called in the new thread
int thread_run(void *data){
	lua_State *Lthread = (lua_State*)data;
	
	if(lua_pcall(Lthread, 0, LUA_MULTRET, 0) != LUA_OK){
		printf("[C] Error in Lua code: %s", lua_tostring(Lthread, -1));
	}
	
	return lua_gettop(Lthread);
}

/* Lua API definitions */

// Creates a new thread
int thread_new(lua_State *L){
	const char *name = luaL_checkstring(L, 1); // stack: {fn, name}
	if(!lua_isfunction(L, 2)){
		luaL_argerror(L, 2, "expected function");
	}
	
	/* Create new Lua thread and give it its starting function */
	lua_State *Lthread = lua_newthread(L); // stack: {thread, fn, name}
	lua_pushvalue(L, 2); // stack: {fn, thread, fn, name}
	lua_xmove(L, Lthread, 1); // Transfer function to new Lua state
	// stack: {thread, fn, name}
	
	/* Create new hardware / SDL thread */
	SDL_Thread *t = SDL_CreateThread(thread_run, name, Lthread);
	checkSDL(t, "Could not create thread: %s");
	
	/* Put hardware / SDL thread into Lua thread's metatable */
	lua_pushlightuserdata(L, t); // stack: {t, thread, fn, name}
	lua_setfield(L, LUA_REGISTRYINDEX, "thread"); // stack: {thread, fn, name}
	
	return 1;
}

// Waits for a thread to complete
int thread_wait(lua_State *L){
	if(!lua_isthread(L, 1)){
		luaL_argerror(L, 1, "expected thread");
	}
	lua_State *Lthread = lua_tothread(L, 1); // stack: {thread}
	lua_getfield(L, LUA_REGISTRYINDEX, "thread"); // stack: {t, thread}
	SDL_Thread *t = (SDL_Thread*)lua_touserdata(L, -1);
	lua_pop(L, 2); // stack: {}
	
	/* Wait for thread */
	int n_return_values;
	printf("[C] Waiting for thread %p\n", t);
	SDL_WaitThread(t, &n_return_values);
	t = NULL; // After SDL_WaitThread, t pointer is invalid
	
	/* Copy its return values and return them */
	lua_xmove(Lthread, L, n_return_values);
	return n_return_values;
}

int luaopen_thread(lua_State *L){
	lua_newtable(L); // stack: {table, ...}
	luaL_setfuncs(L, thread_f, 0);
	return 1;
}