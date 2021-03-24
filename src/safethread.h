#pragma once

#include <lua.h>

#include "threads.h"

/* C library definitions */

typedef enum ThreadState {
	THREAD_INIT,   // Thread has not yet run
	THREAD_ACTIVE, // Thread is active
	THREAD_DEAD,   // Thread has stopped
} ThreadState;

typedef struct Thread {
	lua_State *L;
	ThreadState state;
	THREAD thread;
	MUTEX mutex;
	CONDITION cond;
} Thread;

/* Lua API definitions */

// Create a new thread
int safethread_new(lua_State *L);

// Sleep the current thread for an amount of time
int safethread_sleep(lua_State *L);

// Exit the current thread
int safethread_exit(lua_State *L);

// Get the current thread
int safethread_self(lua_State *L);

// Wait for a thread to complete
int safethread_wait(lua_State *L);

// Immediately stop a thread
int safethread_kill(lua_State *L);

// Execute a function in a thread
int safethread_pcall(lua_State *L);

// Push an event on the queue in the thread
int safethread_pushEvent(lua_State *L);

LUAMOD_API int luaopen_safethread(lua_State *L);