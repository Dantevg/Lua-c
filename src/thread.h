#pragma once

#include <lua.h>
#include <lauxlib.h>

/* C library definitions */

/* Lua API definitions */

// Create a new thread
int thread_new(lua_State *L);

// Wait for a thread to complete
int thread_wait(lua_State *L);

// Immediately stop a thread
int thread_kill(lua_State *L);

LUAMOD_API int luaopen_thread(lua_State *L);