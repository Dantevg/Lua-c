#include <SDL.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "main.h"

/* C library definitions */

// Callback function which gets called in different thread
uint32_t timer_async_callback(uint32_t delay, void *param){
	Timer *timer = param;
	
	SDL_Event event;
	SDL_UserEvent userevent;
	
	userevent.type = SDL_USEREVENT;
	userevent.code = 1;
	userevent.data1 = param;
	
	event.type = SDL_USEREVENT;
	event.user = userevent;
	
	// Push timer event on the event stack,
	// to call the lua function in the main thread
	SDL_PushEvent(&event);
	
	// Return the delay to continue repeating, return 0 to stop
	return (timer->repeat) ? delay : 0;
}

/* Lua API definitions */

int event_addTimer(lua_State *L){
	// I have to make this static (or put it on the heap),
	// as it otherwise gets deallocated at the end of this function,
	// and the callback wouldn't be able to use it anymore.
	static Timer timer;
	timer.delay = luaL_checkinteger(L, 1);
	lua_pushvalue(L, 2);
	luaL_checktype(L, 2, LUA_TFUNCTION);
	timer.fn = luaL_ref(L, LUA_REGISTRYINDEX);
	// Set repeat to argument if argument present, otherwise set to true
	timer.repeat = lua_toboolean(L, 3);
	
	SDL_AddTimer(timer.delay, timer_async_callback, &timer);
	lua_pushinteger(L, timer.fn);
	return 1;
}

int event_on(lua_State *L){
	const char *event = luaL_checkstring(L, 1); // stack: {callback, event}
	lua_getfield(L, LUA_REGISTRYINDEX, "events"); // stack: {callbacks, callback, event}
	
	/* Increment n */
	lua_getfield(L, -1, "n"); // stack: {n, callbacks, callback, event}
	int n = lua_tointeger(L, -1);
	lua_pushinteger(L, ++n); // stack: {n+1, n, callbacks, callback, event}
	lua_replace(L, -2); // stack: {n+1, callbacks, callback, event}
	lua_setfield(L, -2, "n"); // stack: {callbacks, callback, event}
	
	/* Put callback in registry */
	lua_pushvalue(L, 2); // stack: {callback, callbacks, callback, event}
	int id = luaL_ref(L, LUA_REGISTRYINDEX); // stack: {callbacks, callback, event}
	Callback *callback = lua_newuserdata(L, sizeof(Callback)); // stack: {callback userdata, callbacks, callback, event}
	callback->fn = id; // TODO: possibility to add extra data
	callback->event = event;
	lua_rawseti(L, 3, n); // stack: {callbacks, callback, event}
	lua_pop(L, 3); // stack: {}
	lua_pushinteger(L, n);
	return 1;
}

int event_off(lua_State *L){
	int n = luaL_checkinteger(L, 1); // stack: {n}
	lua_getfield(L, LUA_REGISTRYINDEX, "events"); // stack: {callbacks, n}
	lua_rawgeti(L, -1, n); // stack: {callbacks[n], callbacks, n}
	
	/* Get callback struct */
	void *ptr = lua_touserdata(L, -1);
	if(ptr == NULL){ // No callback present, return false
		lua_pushboolean(L, 0);
		return 1;
	}
	Callback *callback = (Callback*)ptr;
	
	/* Remove callback struct from callback table */
	lua_pushnil(L); // stack: {nil, callbacks[n], callbacks, n}
	lua_rawseti(L, -3, n); // stack: {callbacks[n], callbacks, n}
	
	/* Unreference Lua callback function */
	luaL_unref(L, LUA_REGISTRYINDEX, callback->fn);
	
	lua_pushboolean(L, 1); // Callback successfully removed, return true
	return 1;
}

static const struct luaL_Reg event[] = {
	{"addTimer", event_addTimer},
	{"on", event_on},
	{"off", event_off},
	{NULL, NULL}
};

int luaopen_event(lua_State *L){
	lua_newtable(L);
	luaL_setfuncs(L, event, 0);
	return 1;
}