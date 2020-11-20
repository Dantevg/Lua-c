#pragma once

#include <SDL2/SDL.h>

#include <lua.h>
#include <lauxlib.h>

/* C library definitions */

// Gets called in the new thread
int thread_run(void *data);

/* Lua API definitions */

// Creates a new thread
int thread_new(lua_State *L);

// Waits for a thread to complete
int thread_wait(lua_State *L);

LUAMOD_API int luaopen_thread(lua_State *L);