/***
The `event` module provides methods for event handling.

The available events are:

- `kb.down`
- `kb.up`
- `kb.input`
- `mouse.move`
- `mouse.down`
- `mouse.up`
- `mouse.scroll`
- `screen.resize`

@module event
@see kb, mouse
*/

#include <SDL2/SDL.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "util.h"
#include "event.h"

/* C library definitions */

// Callback function which gets called in different thread
// Receives the callback struct, puts it in an event and pushes that into the event queue
uint32_t timer_async_callback(uint32_t delay, void *param){
	Callback *callback = param;
	Timer *timer = callback->data;
	
	SDL_Event e;
	SDL_UserEvent userevent;
	
	userevent.type = SDL_USEREVENT;
	userevent.code = 1;
	userevent.data1 = param;
	
	e.type = SDL_USEREVENT;
	e.user = userevent;
	
	// Push timer event on the event stack,
	// to call the lua function in the main thread
	SDL_PushEvent(&e);
	
	// Return the delay to continue repeating, return 0 to stop
	return (timer->repeat) ? delay : 0;
}

// Creates a callback for the function in the given registry id, for the given event,
// with the given data, and places it into the callback table
// Returns the callback struct, and places the callback id on the stack
Callback *event_add_callback(lua_State *L, int filter_id, int callback_id, void *data){
	lua_getfield(L, LUA_REGISTRYINDEX, "event_callbacks"); // stack: {callbacks, ...}
	
	/* Increment n */
	lua_getfield(L, -1, "n"); // stack: {n, callbacks, ...}
	int n = lua_tointeger(L, -1);
	lua_pushinteger(L, ++n); // stack: {n+1, n, callbacks, ...}
	lua_replace(L, -2); // stack: {n+1, callbacks, ...}
	lua_setfield(L, -2, "n"); // stack: {callbacks, ...}
	
	/* Create callback struct */
	Callback *callback = lua_newuserdata(L, sizeof(Callback)); // stack: {callback userdata, callbacks, ...}
	callback->filter_id = filter_id;
	callback->fn_id = callback_id;
	callback->n = n;
	callback->data = data;
	
	/* Add callback userdata to callback table */
	lua_rawseti(L, -2, n); // stack: {callbacks, ...}
	
	lua_pop(L, 1); // stack: {...}
	lua_pushinteger(L, n); // stack: {n, ...}
	fprintf(stderr, "[C] Registered callback %d, fn %d\n", n, callback->fn_id);
	return callback;
}

int event_match(lua_State *L){
	// TODO: implement
}

// Dispatches event to Lua callbacks
void event_dispatch_callbacks(lua_State *L){
	// stack: {event, ...}
	/* Get callbacks table */
	lua_getfield(L, LUA_REGISTRYINDEX, "event_callbacks"); // stack: {callbacks, event, ...}
	lua_getfield(L, -1, "n"); // stack: {n, callbacks, event, ...}
	int n = lua_tointeger(L, -1);
	lua_pop(L, 1); // stack: {callbacks, event, ...}
	
	/* Loop through all registered callbacks */
	for(int i = 1; i <= n; i++){
		/* Get callback struct from userdata */
		lua_rawgeti(L, -1, i); // stack: {callbacks[i], callbacks, event, ...}
		void *ptr = lua_touserdata(L, -1);
		if(ptr == NULL){ // No callback at this position
			lua_pop(L, 1); // stack: {callbacks, event, ...}
			continue;
		}
		Callback *callback = (Callback*)ptr;
		lua_pop(L, 1); // stack: {callbacks, event, ...}
		
		if(event_match(L)){
			/* Callback is for current event */
			int event_n = luaL_len(L, -3);
			int event_idx = lua_gettop(L) - 1;
			for(int j = 1; j <= event_n; j++){
				lua_geti(L, event_idx, j);
			} // stack: {(eventdata...), callbacks, event, ...}
			
			// TODO: maybe pass callback ID
			lua_rawgeti(L, LUA_REGISTRYINDEX, callback->fn_id); // stack: {fn, (eventdata...), callbacks, event, ...}
			if(lua_pcall(L, event_n, 1, 0) != LUA_OK){ // stack: {error / continue, callbacks, event, ...}
				fprintf(stderr, "%s\n", lua_tostring(L, -1));
			}else if(lua_isboolean(L, -1) && lua_toboolean(L, -1) == 0){
				/* Callback function returned false, remove callback */
				lua_pushcfunction(L, event_off);
				lua_pushinteger(L, i);
				lua_call(L, 1, 0);
			}
			lua_pop(L, 1); // stack: {callbacks, event, ...}
			
			// lua_pushstring(L, eventname);
			// for(int j = 0; j < args; j++){ // Push all arguments on top of stack
			// 	lua_pushvalue(L, -3-args); // stack: {event, fn, callbacks, event, ...}
			// }
			// // TODO: maybe pass callback ID
			// if(lua_pcall(L, args+1, 1, 0) != LUA_OK){ // stack: {error / continue, callbacks, event, ...}
			// 	fprintf(stderr, "%s\n", lua_tostring(L, -1));
			// }else if(lua_isboolean(L, -1) && lua_toboolean(L, -1) == 0){
			// 	/* Callback function returned false, remove callback */
			// 	lua_pushcfunction(L, event_off);
			// 	lua_pushinteger(L, i);
			// 	lua_call(L, 1, 0);
			// }
			// lua_pop(L, 1); // stack: {callbacks, event, ...}
		}
	}
	lua_pop(L, 1); // stack: {...}
}

// Poll for events
void event_poll(lua_State *L){
	SDL_Event e;
	while(SDL_PollEvent(&e)){
		printf("Event: %d\n", e.type);
		if(e.type == SDL_WINDOWEVENT) printf("window\n");
		if(e.type == SDL_QUIT){
			exit(0);
		}else if(e.type == SDL_USEREVENT && e.user.code == 1){
			printf("timer\n");
			Timer *timer = (Timer*)((Callback*)e.user.data1)->data;
			lua_pushstring(L, "timer");
			lua_pushinteger(L, timer->id);
			lua_pushinteger(L, timer->delay);
			lua_pushcfunction(L, event_push);
			if(lua_pcall(L, 3, 1, 0) == LUA_OK && lua_isboolean(L, -1) && lua_toboolean(L, -1) == 0){
				/* Callback function returned false, remove timer callback */
			}
			
			/* Got callback event, call Lua callback */
			// Callback *callback = e.user.data1;
			// lua_rawgeti(L, LUA_REGISTRYINDEX, callback->fn); // stack: {fn, ...}
			// lua_pushstring(L, "timer"); // stack: {"timer", fn, ...}
			// lua_pushinteger(L, ((Timer*)callback->data)->delay); // stack: {delay, "timer", fn, ...}
			
			// if(lua_pcall(L, 2, 1, 0) != LUA_OK){ // stack: {error / continue, ...}
			// 	fprintf(stderr, "%s\n", lua_tostring(L, -1));
			// }else if(lua_isboolean(L, -1) && lua_toboolean(L, -1) == 0){
			// 	/* Callback function returned false, remove timer callback */
			// 	lua_pushcfunction(L, event_removeTimer);
			// 	lua_pushinteger(L, callback->id);
			// 	lua_call(L, 1, 0);
			// }
			// lua_pop(L, 1); // stack: {...}
		}else if(e.type == SDL_KEYDOWN){
			const char *key = SDL_GetKeyName(e.key.keysym.sym);
			int length = strlen(key)+1;
			char keyLower[length];
			lower(key, keyLower, length);
			lua_pushstring(L, "kb"); lua_pushstring(L, "down");
			lua_pushstring(L, keyLower);
			lua_pushcfunction(L, event_push);
			lua_pcall(L, 3, 0, 0);
		}else if(e.type == SDL_KEYUP){
			const char *key = SDL_GetKeyName(e.key.keysym.sym);
			int length = strlen(key)+1;
			char keyLower[length];
			lower(key, keyLower, length);
			lua_pushstring(L, "kb"); lua_pushstring(L, "up");
			lua_pushstring(L, keyLower);
			lua_pushcfunction(L, event_push);
			lua_pcall(L, 3, 0, 0);
		}else if(e.type == SDL_TEXTINPUT){
			lua_pushstring(L, "kb"); lua_pushstring(L, "input");
			lua_pushstring(L, e.text.text);
			lua_pushcfunction(L, event_push);
			lua_pcall(L, 3, 0, 0);
		}else if(e.type == SDL_MOUSEMOTION){
			lua_pushstring(L, "mouse"); lua_pushstring(L, "move");
			lua_pushinteger(L, e.motion.x);
			lua_pushinteger(L, e.motion.y);
			lua_pushinteger(L, e.motion.xrel);
			lua_pushinteger(L, e.motion.yrel);
			lua_pushcfunction(L, event_push);
			lua_pcall(L, 6, 0, 0);
		}else if(e.type == SDL_MOUSEBUTTONDOWN){
			lua_pushstring(L, "mouse"); lua_pushstring(L, "down");
			lua_pushinteger(L, e.button.button);
			lua_pushinteger(L, e.button.x);
			lua_pushinteger(L, e.button.y);
			lua_pushboolean(L, e.button.clicks-1);
			lua_pushcfunction(L, event_push);
			lua_pcall(L, 6, 0, 0);
		}else if(e.type == SDL_MOUSEBUTTONUP){
			lua_pushstring(L, "mouse"); lua_pushstring(L, "up");
			lua_pushinteger(L, e.button.button);
			lua_pushinteger(L, e.button.x);
			lua_pushinteger(L, e.button.y);
			lua_pushboolean(L, e.button.clicks-1);
			lua_pushcfunction(L, event_push);
			lua_pcall(L, 6, 0, 0);
		}else if(e.type == SDL_MOUSEWHEEL){
			lua_pushstring(L, "mouse"); lua_pushstring(L, "scroll");
			lua_pushinteger(L, e.wheel.x);
			lua_pushinteger(L, e.wheel.y);
			lua_pushboolean(L, e.wheel.direction);
			lua_pushcfunction(L, event_push);
			lua_pcall(L, 5, 0, 0);
		}else if(e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_RESIZED){
			printf("[C] resize ");
			lua_pushstring(L, "screen"); lua_pushstring(L, "resize");
			lua_pushinteger(L, e.window.data1);
			lua_pushinteger(L, e.window.data2);
			lua_pushcfunction(L, event_push);
			lua_pcall(L, 4, 0, 0);
		}
	}
}

// Handle events and dispatch them to Lua
int event_loop(lua_State *L){
	uint32_t loop_start = SDL_GetTicks();
	
	/* Poll for SDL events */
	event_poll(L);
	
	/* Handle Lua events */
	lua_getfield(L, LUA_REGISTRYINDEX, "event_queue"); // stack: {queue}
	int n = luaL_len(L, -1);
	for(int i = 1; i <= n; i++){
		lua_geti(L, -1, i); // stack: {event, queue}
		lua_geti(L, -1, 1); printf("handle event %s\n", lua_tostring(L, -1)); lua_pop(L, 1); // TODO: remove
		event_dispatch_callbacks(L); // stack: {queue}
		// lua_getfield(L, -1, "name"); // stack: {name, event, queue}
		// const char *name = lua_tostring(L, -1);
		// lua_pop(L, 1); // stack: {event, queue}
		// int n_args = luaL_len(L, -1);
		// int top = lua_gettop(L);
		// for(int j = 1; j <= n_args; j++){
		// 	lua_geti(L, top, j);
		// } // stack: {v (x n_args), event, queue}
		// event_dispatch_callbacks(L, name, n_args); // stack: {event, queue}
		// lua_pop(L, 1); // stack: {queue}
		lua_pushnil(L);
		lua_seti(L, -2, i);
	}
	lua_pop(L, 1); // stack: {}
	
	if(SDL_GetTicks() < loop_start + 1) SDL_Delay(1);
	
	return 0;
}

/* Lua API definitions */

/***
 * Register event callback.
 * @function on
 * @tparam table filter the event name
 * @tparam function callback the callback function
 * @treturn number the callback id
 */
int event_on(lua_State *L){
	const char *e = luaL_checkstring(L, 1); // stack: {callback, filter}
	int callback_id = luaL_ref(L, LUA_REGISTRYINDEX); // stack: {filter}
	int filter_id = luaL_ref(L, LUA_REGISTRYINDEX); // stack: {}
	event_add_callback(L, filter_id, callback_id, NULL); // stack: {n}
	return 1;
}

/***
 * Deregister event callback.
 * @function off
 * @tparam number id the callback id, as returned by @{event.on}
 * @treturn boolean whether the callback was successfully removed
 */
int event_off(lua_State *L){
	int n = luaL_checkinteger(L, 1); // stack: {n}
	lua_getfield(L, LUA_REGISTRYINDEX, "event_callbacks"); // stack: {callbacks, n}
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

/***
 * Register timer callback.
 * @function addTimer
 * @tparam number delay the delay in milliseconds
 * @tparam function callback the callback function
 * @tparam[opt=false] boolean repeat when `true`, registers a repeating interval.
 * When `false`, triggers only once.
 * @treturn number callback id
 */
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

/***
 * Deregister timer callback.
 * @function removeTimer
 * @tparam number id the callback id, as returned by @{event.addTimer}
 * @treturn boolean whether the timer was successfully removed
 */
int event_removeTimer(lua_State *L){
	/* Remove callback */
	event_off(L); // stack: {status, callbacks[n], callbacks, n}
	if(lua_toboolean(L, -1)){ // Callback correctly removed
		/* Remove SDL timer */
		Callback *callback = (Callback*)lua_touserdata(L, -2);
		Timer *timer = callback->data;
		fprintf(stderr, "[C] Removing timer %d (%s), fn %d\n", (int)lua_tointeger(L, 1), callback->event, callback->fn);
		SDL_RemoveTimer(timer->id);
		free(timer); // malloc'd by event_addTimer
	}
	
	return 1;
}

/***
 * Push an event on the queue.
 * @function push
 * @tparam string name the event name
 * @param[opt] ... the event arguments
 */
int event_push(lua_State *L){
	int n_args = lua_gettop(L)-1; // stack: {(args...)}
	lua_getfield(L, LUA_REGISTRYINDEX, "event_queue"); // stack: {queue, (args...)}
	lua_newtable(L); // stack: {event, queue, (args...)}
	for(int i = 1; i <= n_args; i++){
		lua_seti(L, lua_gettop(L), i);
	} // stack: {event, queue}
	lua_seti(L, 1, luaL_len(L, 1)+1); // stack: {queue}
	return 0;
	
	// lua_rotate(L, 1, -1); // stack: {name, event, events, args}
	// lua_setfield(L, -2, "name"); // stack: {event, events, args}
	// lua_rotate(L, 1, -n_args); // stack: {args, event, events}
	// for(int i = n_args; i >= 1; i--){
	// 	lua_seti(L, 2, i);
	// } // stack: {event, events}
	// lua_seti(L, 1, luaL_len(L, 1)+1); // stack: {events}
	return 0;
}

static const struct luaL_Reg event_f[] = {
	{"on", event_on},
	{"off", event_off},
	{"addTimer", event_addTimer},
	{"removeTimer", event_removeTimer},
	{"push", event_push},
	{NULL, NULL}
};

LUAMOD_API int luaopen_event(lua_State *L){
	lua_newtable(L);
	luaL_setfuncs(L, event_f, 0);
	
	/* Put pointer to event_loop in registry, for main.c to call */
	// lua_pushlightuserdata(L, &event_loop);
	// lua_setfield(L, LUA_REGISTRYINDEX, "event_loop");
	
	/* Register callbacks table */
	lua_newtable(L); // stack: {tbl, ...}
	lua_pushinteger(L, 0); // stack: {0, tbl, ...}
	lua_setfield(L, -2, "n"); // stack: {tbl, ...}
	lua_setfield(L, LUA_REGISTRYINDEX, "event_callbacks"); // stack: {, ...}
	
	/* Register event queue */
	lua_newtable(L);
	lua_setfield(L, LUA_REGISTRYINDEX, "event_queue");
	
	return 1;
}