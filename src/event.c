#include <SDL2/SDL.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "util.h"
#include "main.h"
#include "event.h"

/* C library definitions */

// Callback function which gets called in different thread
// Receives the callback struct, puts it in an event and pushes that into the event queue
uint32_t timer_async_callback(uint32_t delay, void *param){
	Callback *callback = param;
	Timer *timer = callback->data;
	
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

// Creates a callback for the function in the given registry id, for the given event,
// with the given data, and places it into the callback table
// Returns the callback struct, and places the callback id on the stack
Callback *event_add_callback(lua_State *L, const char *event, int callbackid, void *data){
	lua_getfield(L, LUA_REGISTRYINDEX, "callbacks"); // stack: {callbacks, ...}
	
	/* Increment n */
	lua_getfield(L, -1, "n"); // stack: {n, callbacks, ...}
	int n = lua_tointeger(L, -1);
	lua_pushinteger(L, ++n); // stack: {n+1, n, callbacks, ...}
	lua_replace(L, -2); // stack: {n+1, callbacks, ...}
	lua_setfield(L, -2, "n"); // stack: {callbacks, ...}
	
	/* Create callback struct */
	Callback *callback = lua_newuserdata(L, sizeof(Callback)); // stack: {callback userdata, callbacks, ...}
	callback->fn = callbackid;
	callback->event = event;
	callback->data = data;
	callback->id = n;
	
	/* Add callback userdata to callback table */
	lua_rawseti(L, -2, n); // stack: {callbacks, ...}
	
	lua_pop(L, 1); // stack: {...}
	lua_pushinteger(L, n); // stack: {n, ...}
	printf("[C] Registered callback %d for %s, fn %d\n", n, callback->event, callback->fn);
	return callback;
}

// Dispatches event to Lua callbacks
void event_dispatch_callbacks(lua_State *L, char *eventname, int args){
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
			// TODO: maybe pass callback ID
			if(lua_pcall(L, args+1, 1, 0) != LUA_OK){ // stack: {error / continue, callbacks, eventdata, ...}
				printf("%s\n", lua_tostring(L, -1));
			}else if(lua_isboolean(L, -1) && lua_toboolean(L, -1) == 0){
				/* Callback function returned false, remove callback */
				lua_pushcfunction(L, event_off);
				lua_pushinteger(L, i);
				lua_call(L, 1, 0);
			}
			lua_pop(L, 1); // stack: {callbacks, eventdata, ...}
		}
	}
	lua_pop(L, 2); // stack: {...}
}

// Dispatches event to Lua
void event_dispatch(lua_State *L, SDL_Event *event){
	if(event->type == SDL_USEREVENT && event->user.code == 1){
		/* Got callback event, call Lua callback */
		Callback *callback = event->user.data1;
		lua_rawgeti(L, LUA_REGISTRYINDEX, callback->fn); // stack: {fn, ...}
		lua_pushstring(L, "timer"); // stack: {"timer", fn, ...}
		lua_pushinteger(L, ((Timer*)callback->data)->delay); // stack: {delay, "timer", fn, ...}
		
		if(lua_pcall(L, 2, 1, 0) != LUA_OK){ // stack: {error / continue, ...}
			printf("%s\n", lua_tostring(L, -1));
		}else if(lua_isboolean(L, -1) && lua_toboolean(L, -1) == 0){
			/* Callback function returned false, remove timer callback */
			lua_pushcfunction(L, event_removeTimer);
			lua_pushinteger(L, callback->id);
			lua_call(L, 1, 0);
		}
		lua_pop(L, 1); // stack: {...}
	}else if(event->type == SDL_KEYDOWN){
		const char *key = SDL_GetKeyName(event->key.keysym.sym);
		int length = strlen(key)+1;
		char keyLower[length];
		lower(key, keyLower, length);
		lua_pushstring(L, keyLower);
		event_dispatch_callbacks(L, "kb.down", 1);
	}else if(event->type == SDL_KEYUP){
		const char *key = SDL_GetKeyName(event->key.keysym.sym);
		int length = strlen(key)+1;
		char keyLower[length];
		lower(key, keyLower, length);
		lua_pushstring(L, keyLower);
		event_dispatch_callbacks(L, "kb.up", 1);
	}else if(event->type == SDL_TEXTINPUT){
		lua_pushstring(L, event->text.text);
		event_dispatch_callbacks(L, "kb.input", 1);
	}else if(event->type == SDL_MOUSEMOTION){
		lua_pushinteger(L, event->motion.x);
		lua_pushinteger(L, event->motion.y);
		event_dispatch_callbacks(L, "mouse.move", 2);
	}else if(event->type == SDL_MOUSEBUTTONDOWN){
		lua_pushinteger(L, event->button.button);
		lua_pushinteger(L, event->button.x);
		lua_pushinteger(L, event->button.y);
		lua_pushboolean(L, event->button.clicks-1);
		event_dispatch_callbacks(L, "mouse.down", 4);
	}else if(event->type == SDL_MOUSEBUTTONUP){
		lua_pushinteger(L, event->button.button);
		lua_pushinteger(L, event->button.x);
		lua_pushinteger(L, event->button.y);
		lua_pushboolean(L, event->button.clicks-1);
		event_dispatch_callbacks(L, "mouse.up", 4);
	}else if(event->type == SDL_MOUSEWHEEL){
		lua_pushinteger(L, event->wheel.x);
		lua_pushinteger(L, event->wheel.y);
		lua_pushboolean(L, event->wheel.direction);
		event_dispatch_callbacks(L, "mouse.scroll", 3);
	}else if(event->type == SDL_WINDOWEVENT && event->window.event == SDL_WINDOWEVENT_RESIZED){
		lua_pushinteger(L, event->window.data1);
		lua_pushinteger(L, event->window.data2);
		event_dispatch_callbacks(L, "screen.resize", 2);
	}
}

/* Lua API definitions */

// Registers an event callback
// Expects an event name, and a callback function
// Returns the callback id
int event_on(lua_State *L){
	const char *event = luaL_checkstring(L, 1); // stack: {callback, event}
	int id = luaL_ref(L, LUA_REGISTRYINDEX); // stack: {event}
	event_add_callback(L, event, id, NULL); // stack: {n, event}
	return 1;
}

// Deregisters an event callback
// Expects a callback id, as returned by event_on
// Returns whether the callback was successfully removed
int event_off(lua_State *L){
	int n = luaL_checkinteger(L, 1); // stack: {n}
	lua_getfield(L, LUA_REGISTRYINDEX, "callbacks"); // stack: {callbacks, n}
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

// Adds a timer callback
// Expects a delay in milliseconds, a callback function, and optionally a boolean repeat
// Returns the callback id
int event_addTimer(lua_State *L){
	luaL_checktype(L, 2, LUA_TFUNCTION); // stack: {(repeat?), callback, delay}
	
	/* Create timer */
	Timer *timer = malloc(sizeof(Timer)); // free'd by event_removeTimer
	timer->delay = luaL_checkinteger(L, 1);
	timer->repeat = lua_toboolean(L, 3);
	
	/* Register timer callback event */
	lua_pushvalue(L, 2); // stack: {callback, (repeat?), callback, delay}
	int id = luaL_ref(L, LUA_REGISTRYINDEX); // stack: {(repeat?), callback, delay}
	Callback *callback = event_add_callback(L, "timer", id, timer); // stack: {n, (repeat?), callback, delay}
	
	/* Set timer callback id */
	timer->id = SDL_AddTimer(timer->delay, timer_async_callback, callback);
	
	return 1;
}

// Deregisters a timer callback
// Expects a callback id
// Returns whether the timer was successfully removed
int event_removeTimer(lua_State *L){
	/* Remove callback */
	event_off(L); // stack: {status, callbacks[n], callbacks, n}
	if(lua_toboolean(L, -1)){ // Callback correctly removed
		/* Remove SDL timer */
		Callback *callback = (Callback*)lua_touserdata(L, -2);
		Timer *timer = callback->data;
		printf("[C] Removing timer %lld (%s), fn %d\n", lua_tointeger(L, 1), callback->event, callback->fn);
		SDL_RemoveTimer(timer->id);
		free(timer); // malloc'd by event_addTimer
	}
	
	return 1;
}

int luaopen_event(lua_State *L){
	lua_newtable(L);
	luaL_setfuncs(L, event, 0);
	
	/* Put pointer to event_dispatch in registry, for main.c to call */
	lua_pushlightuserdata(L, &event_dispatch);
	lua_setfield(L, LUA_REGISTRYINDEX, "event_dispatch");
	
	return 1;
}