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

#include <string.h>
#include <sched.h> // for yielding in event_loop

#include <SDL2/SDL.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "util.h"
#include "event.h"
#include "table.h"
#include "safethread.h"

/* C library definitions */

// Get the callback struct from the registry
Callback *event_get_callback(lua_State *L, int idx){
	lua_getfield(L, LUA_REGISTRYINDEX, "event_callbacks"); // stack: {callbacks, ...}
	lua_rawgeti(L, -1, idx); // stack: {callbacks[idx], callbacks, ...}
	Callback *callback = (Callback*)lua_touserdata(L, -1);
	lua_pop(L, 2); // stack: {...}
	return callback;
}

// Creates a callback for the function in the given registry id, for the given event,
// with the given data, and places it into the callback table
// Returns the callback struct, and places the callback id on the stack
Callback *event_add_callback(lua_State *L, int filter_id, int callback_id, void *data){
	lua_getfield(L, LUA_REGISTRYINDEX, "event_callbacks"); // stack: {callbacks, ...}
	
	/* Create callback struct */
	Callback *callback = lua_newuserdata(L, sizeof(Callback)); // stack: {callback userdata, callbacks, ...}
	callback->filter_id = filter_id;
	callback->fn_id = callback_id;
	callback->data = data;
	
	/* Add callback userdata to callback table */
	callback->n = table_insert(L, -2);
	
	lua_pop(L, 1); // stack: {...}
	lua_pushinteger(L, callback->n); // stack: {n, ...}
	fprintf(stderr, "[C] Registered callback %d, fn %d\n", callback->n, callback->fn_id);
	return callback;
}

// Match an event filter with an event
int event_match(lua_State *L, Callback *callback){
	lua_rawgeti(L, LUA_REGISTRYINDEX, callback->filter_id); // stack: {filter, callbacks, event, ...}
	int filter_idx = lua_gettop(L);
	int event_idx = lua_gettop(L) - 2;
	int filter_len = luaL_len(L, filter_idx);
	int event_len = luaL_len(L, event_idx);
	
	if(filter_len <= event_len){
		// min(filter_len, event_len) == filter_len, because filter_len <= event_len
		for(int i = 1; i <= filter_len; i++){
			lua_geti(L, filter_idx, i);
			lua_geti(L, event_idx, i);
			if(!lua_compare(L, -1, -2, LUA_OPEQ)){
				lua_pop(L, 3);
				return 0;
			}
			lua_pop(L, 2);
		}
	}
	
	lua_pop(L, 1); // stack: {callbacks, event, ...}
	return 1;
}

// Dispatch single Lua callback
void event_dispatch_callback(lua_State *L, Callback *callback, int i){
	lua_rawgeti(L, LUA_REGISTRYINDEX, callback->filter_id); // stack: {filter, callbacks, event, ...}
	int filter_n = luaL_len(L, -1);
	lua_pop(L, 1); // stack: {callbacks, event, ...}
	
	lua_rawgeti(L, LUA_REGISTRYINDEX, callback->fn_id); // stack: {fn, callbacks, event, ...}
	int event_n = luaL_len(L, -3);
	int event_idx = lua_gettop(L) - 2;
	for(int j = filter_n+1; j <= event_n; j++){
		lua_geti(L, event_idx, j);
	} // stack: {(eventdata...), fn, callbacks, event, ...}
	
	if(lua_pcall(L, event_n - filter_n, 1, 1) == LUA_OK
		&& lua_isboolean(L, -1) && lua_toboolean(L, -1) == 0){
		/* Callback function returned false, remove callback */
		// stack: {continue, callbacks, event, ...}
		lua_pushcfunction(L, event_off);
		lua_pushinteger(L, i);
		lua_call(L, 1, 0);
	}
	lua_pop(L, 1); // stack: {callbacks, event, ...}
}

// Dispatches event to Lua callbacks
void event_dispatch_event(lua_State *L){
	// stack: {event, ...}
	/* Get callbacks table */
	lua_getfield(L, LUA_REGISTRYINDEX, "event_callbacks"); // stack: {callbacks, event, ...}
	int n = table_getn(L, -1);
	
	/* Loop through all registered callbacks */
	for(int i = 1; i <= n; i++){
		/* Get callback struct from userdata */
		lua_rawgeti(L, -1, i); // stack: {callbacks[i], callbacks, event, ...}
		void *ptr = lua_touserdata(L, -1);
		lua_pop(L, 1); // stack: {callbacks, event, ...}
		if(ptr == NULL) continue; // No callback at this position
		Callback *callback = (Callback*)ptr;
		
		// Callback is for current event
		if(event_match(L, callback)) event_dispatch_callback(L, callback, i);
	}
	lua_pop(L, 2); // stack: {...}
}

// Poll for events
void event_poll(lua_State *L){
	/* Poll for timers */
	uint32_t tick = SDL_GetTicks();
	lua_getfield(L, LUA_REGISTRYINDEX, "event_timers"); // stack: {timers, ...}
	int n = table_getn(L, -1);
	for(int i = 1; i <= n; i++){
		if(lua_geti(L, -1, i) == LUA_TNIL){ // stack: {Timer, timers, ...}
			lua_pop(L, 1); // stack: {timers, ...}
			continue;
		}
		Timer *timer = lua_touserdata(L, -1);
		if(timer != NULL && timer->time + timer->delay <= tick){
			lua_pushcfunction(L, event_push);
			lua_pushstring(L, "timer");
			lua_pushinteger(L, i);
			lua_pushinteger(L, tick - timer->time);
			lua_call(L, 3, 0);
			
			if(timer->repeat){
				timer->time = timer->time + timer->delay;
				
				// Don't set next fire to past
				if(timer->time + timer->delay < tick) timer->time = tick - timer->delay;
			}else{
				// Stop non-repeating timer
				lua_pushcfunction(L, event_stopTimer);
				lua_pushinteger(L, i);
				lua_call(L, 1, 0);
			}
		}
		lua_pop(L, 1); // stack: {timers, ...}
	}
	lua_pop(L, 1); // stack: {...}
	
	/* Poll for SDL events */
	SDL_Event e;
	while(SDL_PollEvent(&e)){
		if(e.type == SDL_QUIT){
			exit(0);
		}else if(e.type == SDL_KEYDOWN){
			lua_pushcfunction(L, event_push);
			const char *key = SDL_GetKeyName(e.key.keysym.sym);
			int length = strlen(key)+1;
			char keyLower[length];
			lower(key, keyLower, length);
			lua_pushstring(L, "kb"); lua_pushstring(L, "down");
			lua_pushstring(L, keyLower);
			lua_call(L, 3, 0);
		}else if(e.type == SDL_KEYUP){
			lua_pushcfunction(L, event_push);
			const char *key = SDL_GetKeyName(e.key.keysym.sym);
			int length = strlen(key)+1;
			char keyLower[length];
			lower(key, keyLower, length);
			lua_pushstring(L, "kb"); lua_pushstring(L, "up");
			lua_pushstring(L, keyLower);
			lua_call(L, 3, 0);
		}else if(e.type == SDL_TEXTINPUT){
			lua_pushcfunction(L, event_push);
			lua_pushstring(L, "kb"); lua_pushstring(L, "input");
			lua_pushstring(L, e.text.text);
			lua_call(L, 3, 0);
		}else if(e.type == SDL_MOUSEMOTION){
			lua_pushcfunction(L, event_push);
			lua_pushstring(L, "mouse"); lua_pushstring(L, "move");
			lua_pushinteger(L, e.motion.x);
			lua_pushinteger(L, e.motion.y);
			lua_pushinteger(L, e.motion.xrel);
			lua_pushinteger(L, e.motion.yrel);
			lua_call(L, 6, 0);
		}else if(e.type == SDL_MOUSEBUTTONDOWN){
			lua_pushcfunction(L, event_push);
			lua_pushstring(L, "mouse"); lua_pushstring(L, "down");
			lua_pushinteger(L, e.button.button);
			lua_pushinteger(L, e.button.x);
			lua_pushinteger(L, e.button.y);
			lua_pushboolean(L, e.button.clicks-1);
			lua_call(L, 6, 0);
		}else if(e.type == SDL_MOUSEBUTTONUP){
			lua_pushcfunction(L, event_push);
			lua_pushstring(L, "mouse"); lua_pushstring(L, "up");
			lua_pushinteger(L, e.button.button);
			lua_pushinteger(L, e.button.x);
			lua_pushinteger(L, e.button.y);
			lua_pushboolean(L, e.button.clicks-1);
			lua_call(L, 6, 0);
		}else if(e.type == SDL_MOUSEWHEEL){
			lua_pushcfunction(L, event_push);
			lua_pushstring(L, "mouse"); lua_pushstring(L, "scroll");
			lua_pushinteger(L, e.wheel.x);
			lua_pushinteger(L, e.wheel.y);
			lua_pushboolean(L, e.wheel.direction);
			lua_call(L, 5, 0);
		}else if(e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_RESIZED){
			lua_pushcfunction(L, event_push);
			lua_pushstring(L, "screen"); lua_pushstring(L, "resize");
			lua_pushinteger(L, e.window.data1);
			lua_pushinteger(L, e.window.data2);
			lua_call(L, 4, 0);
		}
	}
}

// Handle events and dispatch them to Lua
int event_loop(lua_State *L){
	uint32_t loop_start = SDL_GetTicks();
	
	if(lua_getfield(L, LUA_REGISTRYINDEX, "event_queue") == LUA_TNIL) return 1; // stack: {queue}
	
	/* Poll for SDL events */
	event_poll(L);
	
	/* Handle Lua events */
	for(int i = 1; i <= table_getn(L, -1); i++){
		lua_geti(L, -1, i); // stack: {event, queue, ...}
		event_dispatch_event(L); // stack: {queue, ...}
		table_remove(L, -1, i);
		lua_pop(L, 1);
	}
	lua_pop(L, 1); // stack: {}
	
	/* Prevent too high cpu usage (SDL_Delay), and let other threads (if any)
	put events in the queue (unlock_mutex, sched_yield) */
	lua_getfield(L, LUA_REGISTRYINDEX, "mb_thread");
	Thread *t = lua_touserdata(L, -1);
	lua_pop(L, 1);
	if(t) unlock_mutex(&t->mutex); // unlock mutex for other threads
	if(SDL_GetTicks() < loop_start + 1){
		SDL_Delay(1);
	}else{
		sched_yield(); // move this thread to end of OS thread queue
	}
	if(t) lock_mutex(&t->mutex); // lock mutex again
	
	return 0;
}

/* Lua API definitions */

/***
 * Register event callback.
 * @function on
 * @param[opt] filter the event filter
 * @param[optchain] ... rest of the filter
 * @tparam function callback the callback function
 * @treturn number the callback id
 */
int event_on(lua_State *L){
	// stack: {callback, (filter...)}
	/* Put callback function into registry */
	int callback_id = luaL_ref(L, LUA_REGISTRYINDEX); // stack: {(filter...)}
	
	/* Put filter vararg into table */
	int filter_len = lua_gettop(L);
	lua_createtable(L, filter_len, 0); // stack: {table, (filter...)}
	lua_rotate(L, 1, 1); // stack: {(filter...), table}
	for(int i = filter_len; i >= 1; i--){
		lua_seti(L, 1, i); // stack: {(filter...), table}
	} // stack: {table}
	
	/* Put filter table into registry */
	int filter_id = luaL_ref(L, LUA_REGISTRYINDEX); // stack: {}
	
	/* Set callback */
	event_add_callback(L, filter_id, callback_id, NULL); // stack: {n}
	return 1;
}

/***
 * Deregister event callback.
 * @function off
 * @tparam number id the callback id, as returned by @{on}
 * @treturn boolean whether the callback was successfully removed
 */
int event_off(lua_State *L){
	int n = luaL_checkinteger(L, 1); // stack: {n}
	
	/* Get callback struct */
	Callback *callback = event_get_callback(L, n);
	if(callback == NULL){ // No callback present, return false
		lua_pushboolean(L, 0);
		return 1;
	}
	
	/* Remove callback struct from callback table */
	lua_pushnil(L); // stack: {nil, callbacks[n], callbacks, n}
	lua_rawseti(L, -3, n); // stack: {callbacks[n], callbacks, n}
	
	/* Unreference Lua callback function and filter table */
	luaL_unref(L, LUA_REGISTRYINDEX, callback->fn_id);
	luaL_unref(L, LUA_REGISTRYINDEX, callback->filter_id);
	
	lua_pushboolean(L, 1); // Callback successfully removed, return true
	return 1;
}

/***
 * Start a timer.
 * @function startTimer
 * @tparam number delay the delay in milliseconds
 * @tparam[opt=false] boolean repeat when `true`, registers a repeating interval.
 * When `false`, triggers only once.
 * @treturn number timer id
 */
int event_startTimer(lua_State *L){
	// stack: {(repeat?), delay}
	/* Create timer */
	Timer *timer = malloc(sizeof(Timer)); // free'd by event_stopTimer
	timer->delay = luaL_checkinteger(L, 1);
	timer->repeat = lua_toboolean(L, 2);
	timer->time = SDL_GetTicks();
	
	/* Add Timer struct to event_timers table */
	lua_getfield(L, LUA_REGISTRYINDEX, "event_timers"); // stack: {timers, (repeat?), delay}
	lua_pushlightuserdata(L, timer); // stack: {timer, timers, (repeat?), delay}
	int timer_id = table_insert(L, -2); // stack: {timers, (repeat?), delay}
	
	lua_pushinteger(L, timer_id); // stack: {timer_id, timers, (repeat?), delay}
	return 1;
}

/***
 * Stop a timer.
 * @function stopTimer
 * @tparam number id the timer id, as returned by @{startTimer}
 * @treturn boolean whether the timer was successfully stopped
 */
int event_stopTimer(lua_State *L){
	int id = luaL_checkinteger(L, 1); // stack: {id}
	
	/* Get Timer struct from event_timers table */
	lua_getfield(L, LUA_REGISTRYINDEX, "event_timers"); // stack: {timers, id}
	if(lua_geti(L, -1, id) == LUA_TNIL){ // stack: {Timer, timers, id}
		// No such timer found, return false
		lua_pushboolean(L, 0);
		return 1;
	}
	
	/* Free Timer struct */
	Timer *timer = lua_touserdata(L, -1);
	if(timer != NULL) free(timer); // malloc'd by event_startTimer
	
	table_remove(L, -3, id);
	
	lua_pushboolean(L, 1);
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
	int has_repeat = lua_gettop(L) > 2;
	
	/* Create timer */
	lua_rotate(L, 1, -1); // stack: {delay, (repeat?), callback}
	lua_rotate(L, 2, 1); // stack: {(repeat?), delay, callback}
	lua_pushcfunction(L, event_startTimer); // stack: {event_startTimer, (repeat?), delay, callback}
	lua_rotate(L, 2, 1); // stack: {(repeat?), delay, event_startTimer, callback}
	lua_call(L, has_repeat ? 2 : 1, 1); // stack: {timer_id, callback}
	
	/* Create timer filter */
	lua_createtable(L, 2, 0); // stack: {filter, timer_id, callback}
	lua_pushstring(L, "timer"); // stack: {"timer", filter, timer_id, callback}
	lua_seti(L, -2, 1); // stack: {filter, timer_id, callback}
	lua_rotate(L, 2, 1); // stack: {timer_id, filter, callback}
	lua_seti(L, -2, 2); // stack: {filter, callback}
	
	/* Register timer callback event */
	int filter_id = luaL_ref(L, LUA_REGISTRYINDEX); // stack: {callback}
	int callback_id = luaL_ref(L, LUA_REGISTRYINDEX); // stack: {}
	event_add_callback(L, filter_id, callback_id, NULL); // stack: {n}
	
	return 1;
}

/***
 * Deregister timer callback.
 * @function removeTimer
 * @tparam number id the callback id, as returned by @{addTimer}
 * @treturn boolean whether the timer was successfully removed
 */
int event_removeTimer(lua_State *L){
	/* Remove timer */
	int n = luaL_checkinteger(L, -1); // stack: {n}
	Callback *callback = event_get_callback(L, n);
	lua_geti(L, LUA_REGISTRYINDEX, callback->filter_id); // stack: {filter, n}
	lua_pushcfunction(L, event_stopTimer); // stack: {event_stopTimer, filter, n}
	lua_geti(L, -1, 2); // stack: {timer_id, event_stopTimer, filter, n}
	lua_call(L, 1, 1); // stack: {success, filter, n}
	if(!lua_toboolean(L, -1)){
		return 1; // Timer not stopped, return false from event_stopTimer
	}
	
	/* Remove callback */
	event_off(L); // stack: {status, callbacks[n], callbacks, n}
	return 1;
}

/***
 * Push an event on the queue.
 * @function push
 * @param name the first event argument
 * @param[opt] ... other event arguments
 */
int event_push(lua_State *L){
	int n_args = lua_gettop(L); // stack: {(args...)}
	lua_getfield(L, LUA_REGISTRYINDEX, "event_queue"); // stack: {queue, (args...)}
	lua_newtable(L); // stack: {event, queue, (args...)}
	lua_rotate(L, 1, 2); // stack: {(args...), event, queue}
	for(int i = 1; i <= n_args; i++){
		lua_seti(L, 2, n_args-i+1);
	} // stack: {event, queue}
	table_insert(L, 1); // stack: {queue}
	return 0;
}

int event_print_queue(lua_State *L){
	if(lua_getfield(L, LUA_REGISTRYINDEX, "event_queue") == LUA_TNIL) return 0; // stack: {queue}
	int len = table_getn(L, -1);
	printf("%d events:\t", len);
	
	for(int i = 1; i <= len; i++){
		lua_geti(L, -1, i); // stack: {event, queue}
		if(!lua_istable(L, -1)){
			lua_pop(L, 1); // stack: {queue}
			continue;
		}
		for(int j = 1; j < luaL_len(L, -1); j++){
			lua_geti(L, -1, j); // stack: {event[j], event, queue}
			if(lua_isinteger(L, -1)){
				printf("%d, ", (int)lua_tointeger(L, -1));
			}else if(lua_isnumber(L, -1)){
				printf("%f, ", lua_tonumber(L, -1));
			}else if(lua_isstring(L, -1)){
				printf("%s, ", lua_tostring(L, -1));
			}else{
				printf("%p, ", lua_topointer(L, -1));
			}
			lua_pop(L, 1); // stack: {event, queue}
		}
		lua_pop(L, 1); // stack: {queue}
		printf("\t");
	}
	lua_pop(L, 1); // stack: {}
	printf("\n");
	
	return 0;
}

static const struct luaL_Reg event_f[] = {
	{"on", event_on},
	{"off", event_off},
	{"startTimer", event_startTimer},
	{"stopTimer", event_stopTimer},
	{"addTimer", event_addTimer},
	{"removeTimer", event_removeTimer},
	{"push", event_push},
	{"printQueue", event_print_queue},
	{NULL, NULL}
};

LUAMOD_API int luaopen_event(lua_State *L){
	lua_newtable(L);
	luaL_setfuncs(L, event_f, 0);
	
	/* Register callbacks table */
	table_create(L, 1);
	lua_setfield(L, LUA_REGISTRYINDEX, "event_callbacks"); // stack: {...}
	
	/* Register event queue */
	table_create(L, 1);
	lua_setfield(L, LUA_REGISTRYINDEX, "event_queue"); // stack: {...}
	
	/* Register timer table */
	table_create(L, 0);
	lua_setfield(L, LUA_REGISTRYINDEX, "event_timers");
	
	if(SDL_InitSubSystem(SDL_INIT_EVENTS | SDL_INIT_TIMER) != 0){
		luaL_error(L, "Failed to initialise SDL");
	}
	
	return 1;
}