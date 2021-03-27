#pragma once

#include <lua.h>

#include "threads.h"

/* C library definitions */

// Whether to use the thread's env when copying functions
// When 1, skips copying _ENV and replaces it with the thread's global env
// When 0, copies _ENV so globals inside the thread are invisible
#define SHOULD_USE_THREAD_ENV 1

typedef enum ThreadState {
	THREAD_INIT,   // Thread still has to start executing
	THREAD_IDLE,   // lua_State is inactive
	THREAD_ACTIVE, // lua_State has work queued
	THREAD_DEAD,   // Thread has stopped. The OS thread does not exist anymore
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