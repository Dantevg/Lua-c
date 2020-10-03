#include <SDL.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

/* C library definitions */

struct Timer {
	int id;     // The function id in the lua registry
	int delay;  // The delay in ms
	int repeat; // 1 = repeat, 0 = don't repeat
};

struct Callback {
	int fn;
	int data;
};

// Callback function which gets called in different thread
uint32_t timer_async_callback(uint32_t delay, void *param){
	struct Timer *timer = param;
	
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
	// static struct Callback callback;
	
	static struct Timer timer;
	timer.delay = luaL_checkinteger(L, 1);
	lua_pushvalue(L, 2);
	luaL_checktype(L, 2, LUA_TFUNCTION);
	timer.id = luaL_ref(L, LUA_REGISTRYINDEX);
	// Set repeat to argument if argument present, otherwise set to true
	timer.repeat = lua_isboolean(L, 3) ? lua_toboolean(L, 3) : 1;
	
	SDL_AddTimer(timer.delay, timer_async_callback, &timer);
	return 0;
}

int event_on(lua_State *L){
	const char *event = luaL_checkstring(L, 1); // stack: {callback, event}
	if(lua_getfield(L, LUA_REGISTRYINDEX, event) != LUA_TTABLE){ // stack: {eventtable, callback, event}
		luaL_argerror(L, 1, "invalid event");
	}
	lua_pushvalue(L, 2); // stack: {callback, eventtable, callback, event}
	int id = luaL_ref(L, LUA_REGISTRYINDEX); // stack: {eventtable, callback, event}
	struct Callback *callback = lua_newuserdata(L, sizeof(struct Callback));
	// stack: {callback userdata, eventtable, callback, event}
	callback->fn = id; // TODO: possibility to add extra data
	int n = luaL_len(L, -2); // Size of eventtable
	lua_rawseti(L, 3, n+1); // stack: {eventtable, callback, event}
	lua_pop(L, 3); // stack: {...}
	return 0;
}

static const struct luaL_Reg event[] = {
	{"addTimer", event_addTimer},
	{"on", event_on},
	{NULL, NULL}
};

int luaopen_event(lua_State *L){
	lua_newtable(L);
	luaL_setfuncs(L, event, 0);
	return 1;
}