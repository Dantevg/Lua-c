#pragma once

#include <windows.h>

#include <lua.h>
#include <lauxlib.h>

/* C library definitions */

typedef struct Thread {
	lua_State *Lthread;
	DWORD tid;
	HANDLE thandle;
	int state;
} Thread;

// Gets called in the new thread
DWORD WINAPI thread_run(LPVOID data);

/* Lua API definitions */

// Create a new thread
int thread_new(lua_State *L);

// Wait for a thread to complete
int thread_wait(lua_State *L);

// Immediately stop a thread
int thread_kill(lua_State *L);

LUAMOD_API int luaopen_thread_win(lua_State *L);