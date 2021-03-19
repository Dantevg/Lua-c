#pragma once

#include <lua.h>
#include <lauxlib.h>

#include "threads.h"

/* C library definitions */

typedef struct Thread {
	lua_State *L;
	int state;
	THREAD thread;
	MUTEX mutex;
} Thread;

/* Lua API definitions */

// Create a new thread
int safethread_new(lua_State *L);

// Wait for a thread to complete
int safethread_wait(lua_State *L);

// Immediately stop a thread
int safethread_kill(lua_State *L);

LUAMOD_API int luaopen_safethread(lua_State *L);