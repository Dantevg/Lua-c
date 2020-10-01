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

// Callback function which gets called in different thread
uint32_t timer_callback(uint32_t delay, void *param){
	struct Timer *timer = param;
	
	SDL_Event event;
	SDL_UserEvent userevent;
	
	userevent.type = SDL_USEREVENT;
	userevent.code = 1;
	userevent.data1 = timer;
	
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
	static struct Timer timer;
	timer.delay = luaL_checkinteger(L, 1);
	lua_pushvalue(L, 2);
	luaL_checktype(L, 2, LUA_TFUNCTION);
	timer.id = luaL_ref(L, LUA_REGISTRYINDEX);
	// Set repeat to argument if argument present, otherwise set to true
	timer.repeat = lua_isboolean(L, 3) ? lua_toboolean(L, 3) : 1;
	
	SDL_AddTimer(timer.delay, timer_callback, &timer);
	return 0;
}

static const struct luaL_Reg event[] = {
	{"addTimer", event_addTimer},
	{NULL, NULL}
};

int luaopen_event(lua_State *L){
	lua_newtable(L);
	luaL_setfuncs(L, event, 0);
	return 1;
}