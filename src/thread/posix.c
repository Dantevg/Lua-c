/***
 * The `posix` thread module provides simple multithreading access.
 * Note that this module is only available on POSIX systems (i.e. not Windows)
 * 
 * @module thread.posix
 * @see thread
 */

#define _REENTRANT // needed for pthread_kill

#include <pthread.h>
#include <signal.h> // for SIGINT

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "thread.h"

/* C library definitions */

typedef struct Thread {
	lua_State *Lthread;
	pthread_t tid;
	int state;
} Thread;

// Gets called in the new thread
void *thread_run(void *data){
	lua_State *Lthread = (lua_State*)data;
	
	if(lua_pcall(Lthread, lua_gettop(Lthread)-1, LUA_MULTRET, 0) != LUA_OK){
		fprintf(stderr, "[Thread %p] %s\n", Lthread, lua_tostring(Lthread, -1));
		return 0;
	}
	
	pthread_exit(NULL);
}

/* Lua API definitions */

// Create a new thread
int thread_new(lua_State *L){
	luaL_argcheck(L, lua_isfunction(L, 1), 1, "expected function");
	
	/* Create Thread struct / userdata */
	Thread *t = lua_newuserdata(L, sizeof(Thread)); // stack: {t, (args?), fn}
	t->state = 1; // Active
	
	/* Create new Lua thread */
	t->Lthread = lua_newthread(L); // stack: {Lthread, t, (args?), fn}
	lua_pop(L, 1); // stack: {t, (args?), fn}
	
	/* Transfer its function and arguments */
	lua_rotate(L, 1, 1); // stack: {(args?), fn, t}
	lua_xmove(L, t->Lthread, lua_gettop(L) - 1); // stack: {t}
	
	/* Create hardware thread */
	pthread_create(&t->tid, NULL, thread_run, t->Lthread);
	
	luaL_setmetatable(L, "Thread");
	return 1;
}

// Wait for a thread to complete
int thread_wait(lua_State *L){
	/* Get Lua thread */
	Thread *t = luaL_checkudata(L, 1, "Thread"); // stack: {t}
	if(t->state == 0) return 0; // Don't wait for a thread that has already stopped
	
	/* Wait for thread and get its return values */
	pthread_join(t->tid, NULL);
	int n_return_values = lua_gettop(t->Lthread);
	lua_xmove(t->Lthread, L, n_return_values);
	t->state = 0; // Inactive
	
	return n_return_values;
}

// Immediately stop a thread
int thread_kill(lua_State *L){
	/* Get Lua thread */
	Thread *t = luaL_checkudata(L, 1, "Thread"); // stack: {t}
	if(t->state == 0) return 0; // Don't wait for a thread that has already stopped
	
	/* Send SIGINT to thread */
	pthread_kill(t->tid, SIGINT);
	t->state = 0; // Inactive
	
	return 0;
}

static const struct luaL_Reg thread_f[] = {
	{"new", thread_new},
	{"wait", thread_wait},
	{"kill", thread_kill},
	{NULL, NULL}
};

LUAMOD_API int luaopen_thread_posix(lua_State *L){
	lua_newtable(L); // stack: {table}
	luaL_setfuncs(L, thread_f, 0);
	
	/* Create Thread metatable */
	if(!luaL_newmetatable(L, "Thread")){ // stack: {mt, table, ...}
		luaL_error(L, "couldn't create Thread metatable");
	}
	
	// For object-oriented access
	lua_pushvalue(L, -2); // stack: {table, mt, table}
	lua_setfield(L, -2, "__index"); // stack: {mt, table}
	
	// To ensure the threads stop when they go out of scope (e.g. when the program stops)
	lua_pushcfunction(L, thread_kill); // stack: {thread_kill, mt, table}
	lua_setfield(L, -2, "__gc"); // stack: {mt, table}
	lua_pop(L, 1); // stack: {table}
	
	return 1;
}