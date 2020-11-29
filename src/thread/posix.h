#pragma once

#include <pthread.h>

#include <lua.h>
#include <lauxlib.h>

/* C library definitions */

typedef struct Thread {
	lua_State *Lthread;
	pthread_t tid;
	int state;
} Thread;

// Gets called in the new thread
void *thread_run(void *data);

// Get thread id from Lua thread in the registry, and remove the registry entry
pthread_t thread_remove(lua_State *L);

/* Lua API definitions */

// Creates a new thread
int thread_new(lua_State *L);

// Waits for a thread to complete
int thread_wait(lua_State *L);

// Immediately stops a thread
int thread_kill(lua_State *L);

LUAMOD_API int luaopen_thread_posix(lua_State *L);