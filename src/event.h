#pragma once

#include <stdint.h> // for uint32_t

#include <lua.h>
#include <lauxlib.h>

/* C library definitions */

typedef struct Timer {
	int delay;     // The delay in ms
	int repeat;    // 1 = repeat, 0 = don't repeat
	uint32_t time; // The time at which the timer started (or restarted)
} Timer;

typedef struct Callback {
	int filter_id; // Filter table id in the Lua registry
	int fn_id;     // Callback function id in the Lua registry
	int n;         // Callback struct number in the callbacks table
	void *data;    // Optional extra data
} Callback;

// Get the callback struct from the registry
Callback *event_get_callback(lua_State *L, int idx);

// Creates a callback for the function in the given registry id, for the given event,
// with the given data, and places it into the callback table
// Returns the callback struct, and places the callback id on the stack
Callback *event_add_callback(lua_State *L, int filter_id, int callback_id, void *data);

// Match an event filter with an event
int event_match(lua_State *L, Callback *callback);

// Dispatches event to Lua callbacks
void event_dispatch_callbacks(lua_State *L);

// Poll for events
void event_poll(lua_State *L);

// Handle events and dispatch them to Lua
int event_loop(lua_State *L);

/* Lua API definitions */

// Registers an event callback
// Expects an event name, and a callback function
// Returns the callback id
int event_on(lua_State *L);

// Deregisters an event callback
// Expects a callback id, as returned by event_on
// Returns whether the callback was successfully removed
int event_off(lua_State *L);

// Starts a timer
// Expects a delay in milliseconds and optionally a boolean repeat
// Returns the timer id
int event_startTimer(lua_State *L);

// Stops a timer
// Expects a timer id
// Returns whether it stopped the timer
int event_stopTimer(lua_State *L);

// Adds a timer callback
// Expects a delay in milliseconds, a callback function, and optionally a boolean repeat
// Returns the callback id
int event_addTimer(lua_State *L);

// Deregisters a timer callback
// Expects a callback id
// Returns whether the timer was successfully removed
int event_removeTimer(lua_State *L);

// Adds an event to the queue
int event_push(lua_State *L);

LUAMOD_API int luaopen_event(lua_State *L);